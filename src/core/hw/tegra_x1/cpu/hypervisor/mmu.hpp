#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/memory.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

constexpr uptr KERNEL_REGION_BASE =
    static_cast<uptr>((long)-(1l << 39)); // TODO: wht

class Mmu : public IMmu {
  public:
    Mmu();
    ~Mmu() override;

    void Map(vaddr_t dst_va, Range<uptr> range,
             const horizon::kernel::MemoryState state) override;
    void Map(vaddr_t dst_va, Range<vaddr_t> range) override;
    void Unmap(Range<vaddr_t> range) override;
    void Protect(Range<vaddr_t> range,
                 horizon::kernel::MemoryPermission perm) override;

    void ResizeHeap(IMemory* heap_mem, vaddr_t va, usize size) override;

    uptr UnmapAddr(vaddr_t va) const override;
    MemoryRegion QueryRegion(vaddr_t va) const override;
    void SetMemoryAttribute(Range<vaddr_t> range,
                            horizon::kernel::MemoryAttribute mask,
                            horizon::kernel::MemoryAttribute value) override;

  protected:
    // Write tracking
    void SetWriteTrackingEnabled(Range<vaddr_t> range, bool enable) override;
    bool TrySuspendWriteTracking(Range<vaddr_t> range) override;
    void ResumeWriteTracking(Range<vaddr_t> range) override;

  private:
    PageTable user_page_table;

  public:
    CONST_REF_GETTER(user_page_table, GetUserPageTable);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
