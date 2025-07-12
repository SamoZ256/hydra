#pragma once

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

class Mmu : public IMmu {
  public:
    void Map(vaddr_t va, usize size, IMemory* memory,
             const horizon::kernel::MemoryState state) override;
    void Map(vaddr_t dst_va, vaddr_t src_va, usize size) override;
    void Unmap(vaddr_t va, usize size) override;

    void ResizeHeap(IMemory* heap_mem, vaddr_t va, usize size) override;

    uptr UnmapAddr(vaddr_t va) const override;
    MemoryRegion QueryRegion(vaddr_t va) const override;

  private:
    uptr pages[128u * 1024u * 1024u] = {0x0};
    horizon::kernel::MemoryState states[128u * 1024u * 1024u] = {
        {.type = horizon::kernel::MemoryType::Free}}; // TODO: handle this
                                                      // differently
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
