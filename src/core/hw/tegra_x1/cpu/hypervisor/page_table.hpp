#pragma once

#include "core/horizon/const.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/page_allocator.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class PageAllocator;

constexpr usize BLOCK_SHIFT_DIFF = 9;
constexpr u64 ENTRY_COUNT = 1ull << BLOCK_SHIFT_DIFF;

#define GET_BLOCK_SHIFT(level) (3 + (BLOCK_SHIFT_DIFF * (3 - (level))))

// TODO: correct?
constexpr u64 ADDRESS_SPACE_SIZE = 1ull << GET_BLOCK_SHIFT(-1);

enum class PageFlags : u8 {
    None = 0,
    WriteTrackingEnabled = ZTD_BITL(0),
};
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(PageFlags);

struct PageTableLevel {
    PageTableLevel(u32 level_, const Page page_, const vaddr_t base_va_);

    u64 getBlockSize() const { return 1ul << GET_BLOCK_SHIFT(level); }

    uptr paToIndex(uptr pa) const {
        return (pa - page.pa) >> GET_BLOCK_SHIFT(level);
    }

    u32 vaToIndex(vaddr_t va) const {
        return static_cast<u32>((va - base_va) >> GET_BLOCK_SHIFT(level));
    }

    u64& getEntry(u32 index) const {
        u64* table = reinterpret_cast<u64*>(page.ptr);
        return table[index];
    }

    PageTableLevel* getNextNoNew(u32 index) {
        ASSERT_DEBUG(level < 2, Hypervisor, "Level 2 is the last level");
        return next_levels[index].level;
    }

    const PageTableLevel* getNextNoNew(u32 index) const {
        ASSERT_DEBUG(level < 2, Hypervisor, "Level 2 is the last level");
        return next_levels[index].level;
    }

    PageTableLevel& getNext(PageAllocator& allocator, u32 index);

    u32 getBlockShift() const { return GET_BLOCK_SHIFT(level); }

    horizon::kernel::MemoryState& getLevelState(u32 index) {
        return next_levels[index].state;
    }

    const horizon::kernel::MemoryState& getLevelState(u32 index) const {
        return next_levels[index].state;
    }

    PageFlags& getLevelFlags(u32 index) { return next_levels[index].flags; }

    const PageFlags& getLevelFlags(u32 index) const {
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
    GETTER(level, getLevel);
};

struct PageRegion {
    vaddr_t va;
    paddr_t pa;
    u64 size;
    horizon::kernel::MemoryState state;

    paddr_t unmapAddr(vaddr_t va_) const { return pa + (va_ - va); }
};

class PageTable {
  public:
    explicit PageTable(paddr_t base_pa);
    ~PageTable();

    void map(vaddr_t va, ztd::Range<uptr> range,
             const horizon::kernel::MemoryState state, ApFlags ap_flags);
    void unmap(ztd::Range<vaddr_t> range);

    // State
    PageRegion queryRegion(vaddr_t va) const;
    void setMemoryPermission(ztd::Range<vaddr_t> range,
                             horizon::kernel::MemoryPermission perm,
                             ApFlags ap_flags);
    void setMemoryAttribute(ztd::Range<vaddr_t> range,
                            horizon::kernel::MemoryAttribute mask,
                            horizon::kernel::MemoryAttribute value);

    // Write tracking
    void setWriteTrackingEnabled(ztd::Range<vaddr_t> range, bool enable);
    bool trySuspendWriteTracking(ztd::Range<vaddr_t> range);
    void resumeWriteTracking(ztd::Range<vaddr_t> range);

    paddr_t unmapAddr(vaddr_t va) const;

    paddr_t getBase() const { return allocator.getBase(); }

  private:
    PageAllocator allocator;
    PageTableLevel top_level;

    void mapLevel(PageTableLevel& level, vaddr_t va, paddr_t pa, u64 size,
                  const horizon::kernel::MemoryState state, ApFlags ap_flags);
    void mapLevelNext(PageTableLevel& level, vaddr_t va, paddr_t pa, u64 size,
                      const horizon::kernel::MemoryState state,
                      ApFlags ap_flags);

    void
    iterateRange(ztd::Range<vaddr_t> range,
                 const std::function<void(ztd::Range<vaddr_t>, u64,
                                          const horizon::kernel::MemoryState&,
                                          PageFlags)>& callback) const;
    void modifyRange(ztd::Range<vaddr_t> range,
                     const std::function<void(ztd::Range<vaddr_t>, u64&,
                                              horizon::kernel::MemoryState&,
                                              PageFlags&)>& callback);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
