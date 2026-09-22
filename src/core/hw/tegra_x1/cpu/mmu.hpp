#pragma once

#include <atomic>

#include "core/horizon/kernel/const.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"

namespace hydra {
class System;
}

namespace hydra::hw::tegra_x1::cpu {

struct MemoryRegion {
    vaddr_t va;
    uptr size;
    horizon::kernel::MemoryState state;
};

// This class doesn't really represent a real MMU, but rather the page table, as
// each process has its own
class IMmu {
  public:
    explicit IMmu(System& system_) : system{system_} {}
    virtual ~IMmu() = default;

    virtual void map(vaddr_t dst_va, ztd::Range<uptr> range,
                     const horizon::kernel::MemoryState state) = 0;
    void map(vaddr_t dst_va, IMemory* memory,
             const horizon::kernel::MemoryState state) {
        map(dst_va,
            ztd::Range<uptr>::fromSize(memory->getPtr(), memory->getSize()),
            state);
    }
    virtual void map(vaddr_t dst_va, ztd::Range<vaddr_t> range) = 0;
    virtual void unmap(ztd::Range<vaddr_t> range) = 0;
    virtual void protect(ztd::Range<vaddr_t> range,
                         horizon::kernel::MemoryPermission perm) = 0;

    virtual uptr unmapAddr(vaddr_t va) const = 0;
    virtual MemoryRegion queryRegion(vaddr_t va) const = 0;
    virtual void setMemoryAttribute(ztd::Range<vaddr_t> range,
                                    horizon::kernel::MemoryAttribute mask,
                                    horizon::kernel::MemoryAttribute value) = 0;

    horizon::kernel::MemoryInfo queryMemory(vaddr_t va) const;
    vaddr_t findFreeMemory(ztd::Range<vaddr_t> region, u64 size) const;

    // Write tracking
    void enableWriteTracking(ztd::Range<vaddr_t> range) {
        setWriteTrackingEnabled(range, true);
    }
    void disableWriteTracking(ztd::Range<vaddr_t> range) {
        setWriteTrackingEnabled(range, false);
    }
    bool trackWrite(ztd::Range<vaddr_t> range);
    void flushTrackedPages();

    // Read
    template <typename T>
    bool tryRead(vaddr_t va, T& out_value) const {
        const auto ptr = unmapAddr(va);
        if (ptr == 0x0) [[unlikely]]
            return false;

        out_value = *reinterpret_cast<T*>(ptr);
        return true;
    }

    template <typename T>
    T read(vaddr_t va) const {
        T value;
        ASSERT_DEBUG(tryRead(va, value), Cpu, "Failed to unmap va 0x{:08x}",
                     va);
        return value;
    }

    // Write
    template <typename T>
    bool tryWrite(vaddr_t va, T value) const {
        const auto ptr = unmapAddr(va);
        if (ptr == 0x0) [[unlikely]]
            return false;

        *reinterpret_cast<T*>(ptr) = value;
        return true;
    }

    template <typename T>
    void write(vaddr_t va, T value) const {
        ASSERT_DEBUG(tryWrite(va, value), Cpu, "Failed to unmap va 0x{:08x}",
                     va);
    }

    template <typename T>
    void writeExclusive(vaddr_t va, T value) const {
        auto ptr = unmapAddr(va);
        ASSERT_DEBUG(ptr != 0x0, Cpu, "Failed to unmap va 0x{:08x}", va);
        atomicStore(reinterpret_cast<T*>(ptr), value);
    }

  protected:
    // Write tracking
    virtual void setWriteTrackingEnabled(ztd::Range<vaddr_t> range,
                                         bool enable) = 0;
    virtual bool trySuspendWriteTracking(ztd::Range<vaddr_t> range) = 0;
    virtual void resumeWriteTracking(ztd::Range<vaddr_t> range) = 0;

  private:
    System& system;

    std::mutex write_tracking_mutex;
    std::vector<ztd::Range<vaddr_t>> tracked_pages;
};

} // namespace hydra::hw::tegra_x1::cpu
