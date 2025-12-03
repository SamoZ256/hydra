#pragma once

#include "core/horizon/kernel/const.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

constexpr usize PAGE_COUNT =
    horizon::kernel::ADDRESS_SPACE.end / GUEST_PAGE_SIZE;

class Mmu : public IMmu {
  public:
    void Map(vaddr_t dst_va, uptr ptr, usize size,
             const horizon::kernel::MemoryState state) override;
    void Map(vaddr_t dst_va, vaddr_t src_va, usize size) override;
    void Unmap(vaddr_t va, usize size) override;

    void ResizeHeap(IMemory* heap_mem, vaddr_t va, usize size) override;

    uptr UnmapAddr(vaddr_t va) const override;
    MemoryRegion QueryRegion(vaddr_t va) const override;

    uptr GetPageTablePtr() const { return reinterpret_cast<uptr>(&pages); }

  private:
    uptr pages[PAGE_COUNT] = {0x0};
    horizon::kernel::MemoryState states[PAGE_COUNT] = {
        {.type = horizon::kernel::MemoryType::Free}};
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
