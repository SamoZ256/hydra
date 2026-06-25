#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/memory.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

constexpr uptr KERNEL_REGION_BASE =
    static_cast<uptr>(static_cast<i64>(-(1l << 39))); // TODO: wht

class Mmu : public IMmu {
  public:
    Mmu(System& system);
    ~Mmu() override;

    void Map(vaddr_t dst_va, ztd::Range<uptr> range,
             const horizon::kernel::MemoryState state) override;
    void Map(vaddr_t dst_va, ztd::Range<vaddr_t> range) override;
    void Unmap(ztd::Range<vaddr_t> range) override;
    void Protect(ztd::Range<vaddr_t> range,
                 horizon::kernel::MemoryPermission perm) override;

    uptr UnmapAddr(vaddr_t va) const override;
    MemoryRegion QueryRegion(vaddr_t va) const override;
    void SetMemoryAttribute(ztd::Range<vaddr_t> range,
                            horizon::kernel::MemoryAttribute mask,
                            horizon::kernel::MemoryAttribute value) override;

  protected:
    // Write tracking
    void SetWriteTrackingEnabled(ztd::Range<vaddr_t> range, bool enable) override;
    bool TrySuspendWriteTracking(ztd::Range<vaddr_t> range) override;
    void ResumeWriteTracking(ztd::Range<vaddr_t> range) override;

  private:
    PageTable user_page_table;

  public:
    CONST_REF_GETTER(user_page_table, GetUserPageTable);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
