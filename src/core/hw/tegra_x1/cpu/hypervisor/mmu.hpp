#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/memory.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

constexpr uptr KERNEL_REGION_BASE = (long)-(1l << 39); // TODO: wht

class Mmu : public IMmu {
  public:
    Mmu();
    ~Mmu() override;

    void Map(vaddr_t va, usize size, IMemory* memory,
             const horizon::kernel::MemoryState state) override;
    void Map(vaddr_t dst_va, vaddr_t src_va, usize size) override;
    void Unmap(vaddr_t va, usize size) override;

    void ResizeHeap(IMemory* heap_mem, vaddr_t va, usize size) override;

    uptr UnmapAddr(vaddr_t va) const override;
    MemoryRegion QueryRegion(vaddr_t va) const override;

  private:
    PageTable user_page_table;

  public:
    CONST_REF_GETTER(user_page_table, GetUserPageTable);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
