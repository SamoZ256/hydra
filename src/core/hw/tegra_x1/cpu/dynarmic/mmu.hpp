#pragma once

#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

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

  private:
    uptr pages[128u * 1024u * 1024u] = {0x0};
    horizon::kernel::MemoryState states[128u * 1024u * 1024u] = {
        {.type = horizon::kernel::MemoryType::Free}}; // TODO: handle this
                                                      // differently
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
