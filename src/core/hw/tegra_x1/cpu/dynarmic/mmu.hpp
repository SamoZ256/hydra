#pragma once

#include "core/horizon/kernel/const.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

constexpr u64 PAGE_COUNT =
    horizon::kernel::ADDRESS_SPACE.getEnd() / GUEST_PAGE_SIZE;

class Mmu : public IMmu {
  public:
    using IMmu::IMmu;

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

    uptr GetPageTablePtr() const { return reinterpret_cast<uptr>(&pages); }

  protected:
    // Write tracking
    void SetWriteTrackingEnabled(ztd::Range<vaddr_t> range, bool enable) override {
        // TODO: implement
        (void)range;
        (void)enable;
        ONCE(LOG_FUNC_NOT_IMPLEMENTED(Dynarmic));
    }
    bool TrySuspendWriteTracking(ztd::Range<vaddr_t> range) override {
        // TODO: implement
        (void)range;
        ONCE(LOG_FUNC_NOT_IMPLEMENTED(Dynarmic));
        return false;
    }
    void ResumeWriteTracking(ztd::Range<vaddr_t> range) override {
        // TODO: implement
        (void)range;
        ONCE(LOG_FUNC_NOT_IMPLEMENTED(Dynarmic));
    }

  private:
    uptr pages[PAGE_COUNT] = {0x0};
    horizon::kernel::MemoryState states[PAGE_COUNT] = {
        {.type = horizon::kernel::MemoryType::Free}};
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
