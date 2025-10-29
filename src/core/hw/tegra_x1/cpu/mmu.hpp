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

    virtual void Map(vaddr_t dst_va, uptr ptr, usize size,
                     const horizon::kernel::MemoryState state) = 0;
    void Map(vaddr_t dst_va, IMemory* memory,
             const horizon::kernel::MemoryState state) {
        Map(dst_va, memory->GetPtr(), memory->GetSize(), state);
    }
    virtual void Map(vaddr_t dst_va, vaddr_t src_va, usize size) = 0;
    virtual void Unmap(vaddr_t va, usize size) = 0;
    virtual void ResizeHeap(IMemory* heap_mem, vaddr_t va,
                            usize size) = 0; // TODO: remove this

    virtual uptr UnmapAddr(vaddr_t va) const = 0;
    virtual MemoryRegion QueryRegion(vaddr_t va) const = 0;

    horizon::kernel::MemoryInfo QueryMemory(vaddr_t va) const;

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
};

} // namespace hydra::hw::tegra_x1::cpu
