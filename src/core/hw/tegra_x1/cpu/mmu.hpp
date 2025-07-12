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

    virtual void Map(vaddr_t va, usize size, IMemory* memory,
                     const horizon::kernel::MemoryState state) = 0;
    void Map(vaddr_t va, IMemory* memory,
             const horizon::kernel::MemoryState state) {
        Map(va, memory->GetSize(), memory, state);
    }
    virtual void Map(vaddr_t dst_va, vaddr_t src_va, usize size) = 0;
    virtual void Unmap(vaddr_t va, usize size) = 0;
    virtual void ResizeHeap(IMemory* heap_mem, vaddr_t va, usize size) = 0;

    virtual uptr UnmapAddr(vaddr_t va) const = 0;
    virtual MemoryRegion QueryRegion(vaddr_t va) const = 0;

    horizon::kernel::MemoryInfo QueryMemory(vaddr_t va) const;

    template <typename T>
    T Load(vaddr_t va) const {
        return *reinterpret_cast<T*>(UnmapAddr(va));
    }

    template <typename T>
    void Store(vaddr_t va, T value) const {
        *reinterpret_cast<T*>(UnmapAddr(va)) = value;
    }

    template <typename T>
    void StoreExclusive(vaddr_t va, T value) const {
        auto ptr = reinterpret_cast<T*>(UnmapAddr(va));
        std::atomic<T>* ref = new (ptr) std::atomic<T>(*ptr);
        ref->store(value, std::memory_order_release); // TODO: correct?
    }
};

} // namespace hydra::hw::tegra_x1::cpu
