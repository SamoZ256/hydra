#pragma once

#include <atomic>

#include "core/horizon/kernel/const.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"

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
    virtual ~IMmu() = default;

    virtual void Map(vaddr_t dst_va, Range<uptr> range,
                     const horizon::kernel::MemoryState state) = 0;
    void Map(vaddr_t dst_va, IMemory* memory,
             const horizon::kernel::MemoryState state) {
        Map(dst_va, Range<uptr>::FromSize(memory->GetPtr(), memory->GetSize()),
            state);
    }
    virtual void Map(vaddr_t dst_va, Range<vaddr_t> range) = 0;
    virtual void Unmap(Range<vaddr_t> range) = 0;
    virtual void Protect(Range<vaddr_t> range,
                         horizon::kernel::MemoryPermission perm) = 0;

    virtual void ResizeHeap(IMemory* heap_mem, vaddr_t va,
                            usize size) = 0; // TODO: remove this

    virtual uptr UnmapAddr(vaddr_t va) const = 0;
    virtual MemoryRegion QueryRegion(vaddr_t va) const = 0;
    virtual void SetMemoryAttribute(Range<vaddr_t> range,
                                    horizon::kernel::MemoryAttribute mask,
                                    horizon::kernel::MemoryAttribute value) = 0;

    horizon::kernel::MemoryInfo QueryMemory(vaddr_t va) const;
    vaddr_t FindFreeMemory(Range<vaddr_t> region, usize size) const;

    // Write tracking
    void EnableWriteTracking(Range<vaddr_t> range) {
        SetWriteTrackingEnabled(range, true);
    }
    void DisableWriteTracking(Range<vaddr_t> range) {
        SetWriteTrackingEnabled(range, false);
    }
    bool TrackWrite(Range<vaddr_t> range);
    void FlushTrackedPages();

    // Read
    template <typename T>
    bool TryRead(vaddr_t va, T& out_value) const {
        const auto ptr = UnmapAddr(va);
        if (ptr == 0x0) [[unlikely]]
            return false;

        out_value = *reinterpret_cast<T*>(ptr);
        return true;
    }

    template <typename T>
    T Read(vaddr_t va) const {
        T value;
        ASSERT_DEBUG(TryRead(va, value), Cpu, "Failed to unmap va 0x{:08x}",
                     va);
        return value;
    }

    // Write
    template <typename T>
    bool TryWrite(vaddr_t va, T value) const {
        const auto ptr = UnmapAddr(va);
        if (ptr == 0x0) [[unlikely]]
            return false;

        *reinterpret_cast<T*>(ptr) = value;
        return true;
    }

    template <typename T>
    void Write(vaddr_t va, T value) const {
        ASSERT_DEBUG(TryWrite(va, value), Cpu, "Failed to unmap va 0x{:08x}",
                     va);
    }

    template <typename T>
    void WriteExclusive(vaddr_t va, T value) const {
        auto ptr = UnmapAddr(va);
        ASSERT_DEBUG(ptr != 0x0, Cpu, "Failed to unmap va 0x{:08x}", va);
        atomic_store(reinterpret_cast<T*>(ptr), value);
    }

  protected:
    // Write tracking
    virtual void SetWriteTrackingEnabled(Range<vaddr_t> range, bool enable) = 0;
    virtual bool TrySuspendWriteTracking(Range<vaddr_t> range) = 0;
    virtual void ResumeWriteTracking(Range<vaddr_t> range) = 0;

  private:
    std::mutex write_tracking_mutex;
    std::vector<Range<vaddr_t>> tracked_pages;
};

} // namespace hydra::hw::tegra_x1::cpu
