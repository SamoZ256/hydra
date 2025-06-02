#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"
#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

constexpr uptr KERNEL_REGION_BASE = (long)-(1l << 39); // TODO: wht
constexpr usize KERNEL_REGION_SIZE = 0x10000000;
constexpr usize KERNEL_MEM_SIZE = 0x1000;

constexpr uptr EXCEPTION_TRAMPOLINE_OFFSET = 0x800;

class MMU : public MMUBase {
  public:
    MMU();
    ~MMU() override;

    MemoryBase* AllocateMemory(usize size) override;
    void FreeMemory(MemoryBase* memory) override;
    uptr GetMemoryPtr(MemoryBase* memory) const override;

    void Map(vaddr_t va, usize size, MemoryBase* memory,
             const horizon::kernel::MemoryState state) override;
    void Map(vaddr_t dst_va, vaddr_t src_va, usize size) override;
    void Unmap(vaddr_t va, usize size) override;

    void ResizeHeap(MemoryBase* heap_mem, vaddr_t va, usize size) override;

    uptr UnmapAddr(vaddr_t va) const override;
    MemoryRegion QueryRegion(vaddr_t va) const override;

    // Getters
    const PageTable& GetUserPageTable() const { return user_page_table; }
    const PageTable& GetKernelPageTable() const { return kernel_page_table; }

  private:
    // Page table
    PageTable user_page_table;
    PageTable kernel_page_table;

    // TODO: use a proper allocator
    uptr physical_memory_ptr;
    u64 physical_memory_cur{0};
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
