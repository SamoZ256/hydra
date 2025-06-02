#pragma once

#include "core/horizon/const.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/page_allocator.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class PageAllocator;

constexpr usize BLOCK_SHIFT_DIFF = 9;
constexpr usize ENTRY_COUNT = 1ull << BLOCK_SHIFT_DIFF;

#define GET_BLOCK_SHIFT(level) (3 + (BLOCK_SHIFT_DIFF * (3 - (level))))

// TODO: correct?
constexpr usize ADDRESS_SPACE_SIZE = 1ull << GET_BLOCK_SHIFT(-1);

struct PageTableLevel {
    PageTableLevel(u32 level_, const Page page_, const vaddr_t base_va_);

    usize GetBlockSize() const { return 1ul << GET_BLOCK_SHIFT(level); }

    uptr PaToIndex(uptr pa) const {
        return (pa - page.pa) >> GET_BLOCK_SHIFT(level);
    }

    u32 VaToIndex(vaddr_t va) const {
        return (va - base_va) >> GET_BLOCK_SHIFT(level);
    }

    u64 ReadEntry(u32 index) const {
        const u64* table = reinterpret_cast<const u64*>(page.ptr);
        return table[index];
    }

    void WriteEntry(u32 index, u64 entry) {
        u64* table = reinterpret_cast<u64*>(page.ptr);
        table[index] = entry;
    }

    const PageTableLevel* GetNextNoNew(u32 index) const {
        ASSERT_DEBUG(level < 2, Hypervisor, "Level 2 is the last level");
        return next_levels[index];
    }

    PageTableLevel& GetNext(PageAllocator& allocator, u32 index);

    // Getters
    u32 GetLevel() const { return level; }

    u32 GetBlockShift() const { return GET_BLOCK_SHIFT(level); }

    const horizon::kernel::MemoryState GetLevelState(u32 index) const {
        return level_states[index];
    }

    // Setters
    void SetLevelState(u32 index, const horizon::kernel::MemoryState state) {
        level_states[index] = state;
    }

  private:
    u32 level;
    const Page page;
    const vaddr_t base_va;
    PageTableLevel* next_levels[ENTRY_COUNT] = {nullptr};
    horizon::kernel::MemoryState level_states[ENTRY_COUNT] = {};
};

struct PageRegion {
    vaddr_t va;
    paddr_t pa;
    usize size;
    horizon::kernel::MemoryState state;

    paddr_t UnmapAddr(vaddr_t va_) const { return pa + (va_ - va); }
};

class PageTable {
  public:
    PageTable(paddr_t base_pa);
    ~PageTable();

    void Map(vaddr_t va, paddr_t pa, usize size,
             const horizon::kernel::MemoryState state, ApFlags flags);
    void Unmap(vaddr_t va, usize size);

    PageRegion QueryRegion(vaddr_t va) const;
    paddr_t UnmapAddr(vaddr_t va) const;

    // Getters
    paddr_t GetBase() const { return allocator.GetBase(); }

  private:
    PageAllocator allocator;
    PageTableLevel top_level;

    void MapLevel(PageTableLevel& level, vaddr_t va, paddr_t pa, usize size,
                  const horizon::kernel::MemoryState state, ApFlags flags);
    void MapLevelNext(PageTableLevel& level, vaddr_t va, paddr_t pa, usize size,
                      const horizon::kernel::MemoryState state, ApFlags flags);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
