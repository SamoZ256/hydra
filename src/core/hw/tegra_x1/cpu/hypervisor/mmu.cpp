#include "core/hw/tegra_x1/cpu/hypervisor/mmu.hpp"

#include "core/debugger/debugger.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/memory.hpp"

/*
#define USER_RANGE_MEM_BASE 0x01000000
#define USER_RANGE_MEM_SIZE 0x1000000

#define KERNEL_RANGE_MEM_BASE 0x04000000
#define KERNEL_RANGE_MEM_SIZE 0x1000000
*/

#define CPU (*static_cast<Cpu*>(&CPU_INSTANCE))

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

inline ApFlags to_ap_flags(horizon::kernel::MemoryPermission perm) {
    // HACK
    return ApFlags::UserReadWriteExecuteKernelReadWrite;

    if (any(perm & horizon::kernel::MemoryPermission::Read)) {
        if (any(perm & horizon::kernel::MemoryPermission::Write)) {
            if (any(perm & horizon::kernel::MemoryPermission::Execute)) {
                return ApFlags::UserReadWriteExecuteKernelReadWrite;
            } else {
                return ApFlags::UserReadWriteKernelReadWrite;
            }
        } else {
            if (any(perm & horizon::kernel::MemoryPermission::Execute)) {
                return ApFlags::UserReadExecuteKernelRead;
            } else {
                return ApFlags::UserReadKernelRead;
            }
        }
    } else {
        if (any(perm & horizon::kernel::MemoryPermission::Write)) {
            if (any(perm & horizon::kernel::MemoryPermission::Execute)) {
                return ApFlags::UserReadWriteExecuteKernelReadWrite; // TODO
            } else {
                return ApFlags::UserReadWriteKernelReadWrite; // TODO
            }
        } else {
            if (any(perm & horizon::kernel::MemoryPermission::Execute)) {
                return ApFlags::UserExecuteKernelRead; // TODO
            } else {
                return ApFlags::UserNoneKernelRead; // TODO
            }
        }
    }
}

// TODO: this is a horrible way to handle this
static bool page_table_regions[16] = {false};

paddr_t FindFreePageTableRegion() {
    for (int i = 0; i < 16; i++) {
        if (!page_table_regions[i]) {
            page_table_regions[i] = true;
            return USER_PAGE_TABLE_REGION_BASE + i * PAGE_TABLE_RESERVED_SIZE;
        }
    }

    LOG_FATAL(Hypervisor, "No free page table region found");
    return 0;
}

void ReleasePageTableRegion(paddr_t addr) {
    ASSERT(addr >= USER_PAGE_TABLE_REGION_BASE &&
               addr <
                   USER_PAGE_TABLE_REGION_BASE + 16 * PAGE_TABLE_RESERVED_SIZE,
           Hypervisor, "Invalid page table region address 0x{:08x}", addr);
    page_table_regions[(addr - USER_PAGE_TABLE_REGION_BASE) /
                       PAGE_TABLE_RESERVED_SIZE] = false;
}

} // namespace

Mmu::Mmu() : user_page_table(FindFreePageTableRegion()) {
    // Loader return address
    // TODO: this should be done in a backend agnostic way (perhaps in the
    // kernel?)
    /*
    uptr ret_mem_ptr = physical_memory_ptr + physical_memory_cur;
    user_page_table.Map(0xffff0000, physical_memory_cur, 0x1000,
                        {horizon::kernel::MemoryType::Code,
                         horizon::kernel::MemoryAttribute::None,
                         horizon::kernel::MemoryPermission::Execute},
                        ApFlags::UserExecuteKernelRead);
    physical_memory_cur += 0x1000;

    *reinterpret_cast<u32*>(ret_mem_ptr) = 0xd40000e1; // svcExitProcess
    */
}

Mmu::~Mmu() { ReleasePageTableRegion(user_page_table.GetBase()); }

void Mmu::Map(vaddr_t va, usize size, IMemory* memory,
              const horizon::kernel::MemoryState state) {
    ASSERT_ALIGNMENT(size, GUEST_PAGE_SIZE, Hypervisor, "size");
    user_page_table.Map(
        va, CPU.GetPAMapper().GetPA(static_cast<Memory*>(memory)->GetPtr()),
        size, state, to_ap_flags(state.perm));
}

// HACK: this assumes that the whole src range is stored contiguously in
// physical memory
void Mmu::Map(vaddr_t dst_va, vaddr_t src_va, usize size) {
    const auto region = user_page_table.QueryRegion(src_va);
    paddr_t pa = region.UnmapAddr(src_va);
    user_page_table.Map(dst_va, pa, size, region.state,
                        to_ap_flags(region.state.perm));
}

void Mmu::Unmap(vaddr_t va, usize size) { user_page_table.Unmap(va, size); }

// TODO: just improve this...
void Mmu::ResizeHeap(IMemory* heap_mem, vaddr_t va, usize size) {
    const auto region = user_page_table.QueryRegion(va);
    paddr_t pa = region.UnmapAddr(va);
    user_page_table.Map(va, pa, size, region.state,
                        to_ap_flags(region.state.perm));
}

uptr Mmu::UnmapAddr(vaddr_t va) const {
    return CPU.GetPAMapper().GetPtr(user_page_table.UnmapAddr(va));
}

MemoryRegion Mmu::QueryRegion(vaddr_t va) const {
    auto region = user_page_table.QueryRegion(va);

    return {
        .va = region.va,
        .size = region.size,
        .state = region.state,
    };
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
