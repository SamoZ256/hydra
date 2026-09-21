#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/memory.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

constexpr uptr KERNEL_REGION_BASE =
    static_cast<uptr>(static_cast<i64>(-(1l << 39))); // TODO: wht

class Mmu : public IMmu {
  public:
    explicit Mmu(System& system);
    ~Mmu() override;

    void map(vaddr_t dst_va, ztd::Range<uptr> range,
             const horizon::kernel::MemoryState state) override;
    void map(vaddr_t dst_va, ztd::Range<vaddr_t> range) override;
    void unmap(ztd::Range<vaddr_t> range) override;
    void protect(ztd::Range<vaddr_t> range,
                 horizon::kernel::MemoryPermission perm) override;

    uptr unmapAddr(vaddr_t va) const override;
    MemoryRegion queryRegion(vaddr_t va) const override;
    void setMemoryAttribute(ztd::Range<vaddr_t> range,
                            horizon::kernel::MemoryAttribute mask,
                            horizon::kernel::MemoryAttribute value) override;

  protected:
    // Write tracking
    void setWriteTrackingEnabled(ztd::Range<vaddr_t> range,
                                 bool enable) override;
    bool trySuspendWriteTracking(ztd::Range<vaddr_t> range) override;
    void resumeWriteTracking(ztd::Range<vaddr_t> range) override;

  private:
    PageTable user_page_table;

  public:
    CONST_REF_GETTER(user_page_table, getUserPageTable);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
