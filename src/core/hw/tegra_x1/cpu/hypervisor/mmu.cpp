#include "core/hw/tegra_x1/cpu/hypervisor/mmu.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/memory.hpp"

/*
#define USER_RANGE_MEM_BASE 0x01000000
#define USER_RANGE_MEM_SIZE 0x1000000

#define KERNEL_RANGE_MEM_BASE 0x04000000
#define KERNEL_RANGE_MEM_SIZE 0x1000000
*/

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

inline ApFlags toApFlags(horizon::kernel::MemoryPermission perm) {
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
bool page_table_regions[16] = {false};

paddr_t findFreePageTableRegion() {
    for (u32 i = 0; i < 16; i++) {
        if (!page_table_regions[i]) {
            page_table_regions[i] = true;
            return USER_PAGE_TABLE_REGION_BASE + i * PAGE_TABLE_RESERVED_SIZE;
        }
    }

    LOG_FATAL(Hypervisor, "No free page table region found");
    return 0;
}

void releasePageTableRegion(paddr_t addr) {
    ASSERT(addr >= USER_PAGE_TABLE_REGION_BASE &&
               addr <
                   USER_PAGE_TABLE_REGION_BASE + 16 * PAGE_TABLE_RESERVED_SIZE,
           Hypervisor, "Invalid page table region address 0x{:08x}", addr);
    page_table_regions[(addr - USER_PAGE_TABLE_REGION_BASE) /
                       PAGE_TABLE_RESERVED_SIZE] = false;
}

} // namespace

Mmu::Mmu(System& system)
    : IMmu(system), user_page_table(findFreePageTableRegion()) {
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

Mmu::~Mmu() { releasePageTableRegion(user_page_table.getBase()); }

void Mmu::map(vaddr_t dst_va, ztd::Range<uptr> range,
              const horizon::kernel::MemoryState state) {
    ASSERT_ALIGNMENT(dst_va, GUEST_PAGE_SIZE, Hypervisor, "destination VA");
    ASSERT_ALIGNMENT(range.getSize(), GUEST_PAGE_SIZE, Hypervisor, "size");
    user_page_table.map(dst_va, range, state, toApFlags(state.perm));
}

// HACK: this assumes that the whole src range is stored contiguously in
// physical memory
void Mmu::map(vaddr_t dst_va, ztd::Range<vaddr_t> range) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Hypervisor, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Hypervisor, "end");
    const auto region = user_page_table.queryRegion(range.getBegin());
    paddr_t pa = region.unmapAddr(range.getBegin());
    // TODO: also inherit flags
    user_page_table.map(dst_va, ztd::Range<uptr>::fromSize(pa, range.getSize()),
                        region.state, toApFlags(region.state.perm));
}

void Mmu::unmap(ztd::Range<vaddr_t> range) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Hypervisor, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Hypervisor, "end");
    user_page_table.unmap(range);
}

void Mmu::protect(ztd::Range<vaddr_t> range,
                  horizon::kernel::MemoryPermission perm) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Hypervisor, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Hypervisor, "end");
    user_page_table.setMemoryPermission(range, perm, toApFlags(perm));
}

uptr Mmu::unmapAddr(vaddr_t va) const { return user_page_table.unmapAddr(va); }

MemoryRegion Mmu::queryRegion(vaddr_t va) const {
    auto region = user_page_table.queryRegion(va);

    return {
        .va = region.va,
        .size = region.size,
        .state = region.state,
    };
}

void Mmu::setMemoryAttribute(ztd::Range<vaddr_t> range,
                             horizon::kernel::MemoryAttribute mask,
                             horizon::kernel::MemoryAttribute value) {
    user_page_table.setMemoryAttribute(range, mask, value);
}

void Mmu::setWriteTrackingEnabled(ztd::Range<vaddr_t> range, bool enable) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Hypervisor, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Hypervisor, "end");
    user_page_table.setWriteTrackingEnabled(range, enable);
}

bool Mmu::trySuspendWriteTracking(ztd::Range<vaddr_t> range) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Hypervisor, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Hypervisor, "end");
    return user_page_table.trySuspendWriteTracking(range);
}

void Mmu::resumeWriteTracking(ztd::Range<vaddr_t> range) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Hypervisor, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Hypervisor, "end");
    user_page_table.resumeWriteTracking(range);
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
