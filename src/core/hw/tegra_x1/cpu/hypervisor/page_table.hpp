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

enum class PageFlags : u8 {
    None = 0,
    WriteTrackingEnabled = BITL(0),
};
ENABLE_ENUM_BITWISE_OPERATORS(PageFlags);

struct PageTableLevel {
    PageTableLevel(u32 level_, const Page page_, const vaddr_t base_va_);

    usize GetBlockSize() const { return 1ul << GET_BLOCK_SHIFT(level); }

    uptr PaToIndex(uptr pa) const {
        return (pa - page.pa) >> GET_BLOCK_SHIFT(level);
    }

    u32 VaToIndex(vaddr_t va) const {
        return static_cast<u32>((va - base_va) >> GET_BLOCK_SHIFT(level));
    }

    u64& GetEntry(u32 index) {
        u64* table = reinterpret_cast<u64*>(page.ptr);
        return table[index];
    }

    const u64& GetEntry(u32 index) const {
        const u64* table = reinterpret_cast<const u64*>(page.ptr);
        return table[index];
    }

    PageTableLevel* GetNextNoNew(u32 index) {
        ASSERT_DEBUG(level < 2, Hypervisor, "Level 2 is the last level");
        return next_levels[index].level;
    }

    const PageTableLevel* GetNextNoNew(u32 index) const {
        ASSERT_DEBUG(level < 2, Hypervisor, "Level 2 is the last level");
        return next_levels[index].level;
    }

    PageTableLevel& GetNext(PageAllocator& allocator, u32 index);

    u32 GetBlockShift() const { return GET_BLOCK_SHIFT(level); }

    horizon::kernel::MemoryState& GetLevelState(u32 index) {
        return next_levels[index].state;
    }

    const horizon::kernel::MemoryState& GetLevelState(u32 index) const {
        return next_levels[index].state;
    }

    PageFlags& GetLevelFlags(u32 index) { return next_levels[index].flags; }

    const PageFlags& GetLevelFlags(u32 index) const {
        return next_levels[index].flags;
    }

  private:
    struct NextLevel {
        PageTableLevel* level{nullptr};
        horizon::kernel::MemoryState state{};
        PageFlags flags{};
    };

    u32 level;
    const Page page;
    const vaddr_t base_va;
    std::array<NextLevel, ENTRY_COUNT> next_levels{};

  public:
    GETTER(level, GetLevel);
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

    void Map(vaddr_t va, Range<uptr> range,
             const horizon::kernel::MemoryState state, ApFlags ap_flags);
    void Unmap(Range<vaddr_t> range);

    // State
    PageRegion QueryRegion(vaddr_t va) const;
    void SetMemoryPermission(Range<vaddr_t> range,
                             horizon::kernel::MemoryPermission perm,
                             ApFlags ap_flags);
    void SetMemoryAttribute(Range<vaddr_t> range,
                            horizon::kernel::MemoryAttribute mask,
                            horizon::kernel::MemoryAttribute value);

    // Write tracking
    void SetWriteTrackingEnabled(Range<vaddr_t> range, bool enable);
    bool TrySuspendWriteTracking(Range<vaddr_t> range);
    void ResumeWriteTracking(Range<vaddr_t> range);

    paddr_t UnmapAddr(vaddr_t va) const;

    paddr_t GetBase() const { return allocator.GetBase(); }

  private:
    PageAllocator allocator;
    PageTableLevel top_level;

    void MapLevel(PageTableLevel& level, vaddr_t va, paddr_t pa, usize size,
                  const horizon::kernel::MemoryState state, ApFlags ap_flags);
    void MapLevelNext(PageTableLevel& level, vaddr_t va, paddr_t pa, usize size,
                      const horizon::kernel::MemoryState state,
                      ApFlags ap_flags);

    void IterateRange(
        Range<vaddr_t> range,
        std::function<void(Range<vaddr_t>, u64,
                           const horizon::kernel::MemoryState&, PageFlags)>
            callback) const;
    void
    ModifyRange(Range<vaddr_t> range,
                std::function<void(Range<vaddr_t>, u64&,
                                   horizon::kernel::MemoryState&, PageFlags&)>
                    callback);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
