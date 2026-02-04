#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"

#include "common/type_aliases.hpp"
#include "core/debugger/debugger_manager.hpp"

#define PTE_TYPE_MASK 0x3ull
#define PTE_BLOCK(level) ((level == 2 ? 3ull : 1ull) << 0)
#define PTE_TABLE (3ull << 0)           // For level 0 and 1 descriptors
#define PTE_AF (1ull << 10)             // Access Flag
#define PTE_RW (1ull << 6)              // Read write
#define PTE_INNER_SHEREABLE (3ull << 8) // TODO: wht

/*
#define USER_RANGE_MEM_BASE 0x01000000
#define USER_RANGE_MEM_SIZE 0x1000000

#define KERNEL_RANGE_MEM_BASE 0x04000000
#define KERNEL_RANGE_MEM_SIZE 0x1000000
*/

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

constexpr u64 ENTRY_ADDR_MASK = 0x0000fffffffff000; // TODO: correct?

} // namespace

PageTableLevel::PageTableLevel(u32 level_, const Page page_,
                               const vaddr_t base_va_)
    : level{level_}, page{page_}, base_va{base_va_} {
    u64* table = reinterpret_cast<u64*>(page.ptr);
    for (u32 i = 0; i < ENTRY_COUNT; i++) {
        table[i] = 0x0; // 0x200000000 | PTE_BLOCK(level);
    }
}

PageTableLevel& PageTableLevel::GetNext(PageAllocator& allocator, u32 index) {
    ASSERT_DEBUG(level < 2, Hypervisor, "Level 2 is the last level");

    auto& next = next_levels[index].level;
    if (!next) {
        next = new PageTableLevel(level + 1, allocator.GetNextPage(),
                                  base_va + index * GetBlockSize());
        GetEntry(index) = next->page.pa | PTE_TABLE;
    }

    return *next;
}

PageTable::PageTable(paddr_t base_pa)
    : allocator(base_pa, 1024), top_level(0, allocator.GetNextPage(), 0x0) {}

PageTable::~PageTable() = default;

void PageTable::Map(vaddr_t va, Range<uptr> range,
                    const horizon::kernel::MemoryState state,
                    ApFlags ap_flags) {
    LOG_DEBUG(Hypervisor, "va: {:#x}, range: {:#x}", va, range);

    ASSERT_ALIGNMENT(va, GUEST_PAGE_SIZE, Hypervisor, "va");
    ASSERT_ALIGNMENT(range.GetBegin(), GUEST_PAGE_SIZE, Hypervisor, "begin");
    ASSERT_ALIGNMENT(range.GetEnd(), GUEST_PAGE_SIZE, Hypervisor, "end");

    MapLevel(top_level, va, range.GetBegin(), range.GetSize(), state, ap_flags);
}

void PageTable::Unmap(Range<vaddr_t> range) {
    LOG_FUNC_WITH_ARGS_NOT_IMPLEMENTED(Hypervisor, "range: {:#x}", range);
}

// TODO: use IterateRange
PageRegion PageTable::QueryRegion(vaddr_t va) const {
#define FREE_MEMORY(region_va, region_size)                                    \
    PageRegion {                                                               \
        .va = region_va, .pa = 0x0, .size = region_size,                       \
        .state = {horizon::kernel::MemoryType::Free,                           \
                  horizon::kernel::MemoryAttribute::None,                      \
                  horizon::kernel::MemoryPermission::None},                    \
    }

#ifdef HYDRA_DEBUG
    if (va >= ADDRESS_SPACE_SIZE)
        return FREE_MEMORY(ADDRESS_SPACE_SIZE, 0); // TODO: size
#endif

    u32 index = top_level.VaToIndex(va);
    auto* level = &top_level;
    u64 entry = top_level.GetEntry(index);
    while ((entry & PTE_TYPE_MASK) != PTE_BLOCK(level->GetLevel())) {
        if ((entry & PTE_TYPE_MASK) != PTE_TABLE)
            return FREE_MEMORY(va & ~(level->GetBlockSize() - 1),
                               level->GetBlockSize());

        level = level->GetNextNoNew(index);
        index = level->VaToIndex(va);
        entry = level->GetEntry(index);
    }

    PageRegion region;
    region.va = va & ~(level->GetBlockSize() - 1);
    region.pa = (entry & ENTRY_ADDR_MASK);
    region.size = level->GetBlockSize();
    region.state = level->GetLevelState(index);

    return region;
}

void PageTable::SetMemoryPermission(Range<vaddr_t> range,
                                    horizon::kernel::MemoryPermission perm,
                                    ApFlags ap_flags) {
    ModifyRange(range, [perm, ap_flags]([[maybe_unused]] Range<vaddr_t> range,
                                        u64& entry,
                                        horizon::kernel::MemoryState& state,
                                        [[maybe_unused]] PageFlags flags) {
        if (!any(flags & PageFlags::WriteTrackingEnabled)) {
            entry &= ~AP_FLAGS_MASK;
            entry |= static_cast<u64>(ap_flags);
        }
        state.perm = perm;
    });
}

void PageTable::SetMemoryAttribute(Range<vaddr_t> range,
                                   horizon::kernel::MemoryAttribute mask,
                                   horizon::kernel::MemoryAttribute value) {
    ModifyRange(range, [mask, value]([[maybe_unused]] Range<vaddr_t> range,
                                     [[maybe_unused]] u64& entry,
                                     horizon::kernel::MemoryState& state,
                                     [[maybe_unused]] PageFlags flags) {
        state.attr = (state.attr & ~mask) | (value & mask);
    });
}

void PageTable::SetWriteTrackingEnabled(Range<vaddr_t> range, bool enable) {
    ModifyRange(range,
                [enable]([[maybe_unused]] Range<vaddr_t> range, u64& entry,
                         [[maybe_unused]] horizon::kernel::MemoryState& state,
                         PageFlags& flags) {
                    // AP flags
                    entry &= ~AP_FLAGS_MASK;
                    entry |= static_cast<u64>(
                        enable ? ApFlags::UserReadKernelRead
                               : ApFlags::UserReadWriteKernelReadWrite);

                    // Page flags
                    if (enable)
                        flags |= PageFlags::WriteTrackingEnabled;
                    else
                        flags &= ~PageFlags::WriteTrackingEnabled;
                });
}

bool PageTable::TrySuspendWriteTracking(Range<vaddr_t> range) {
    bool res = false;
    ModifyRange(range, [&res](
                           [[maybe_unused]] Range<vaddr_t> range, u64& entry,
                           [[maybe_unused]] horizon::kernel::MemoryState& state,
                           [[maybe_unused]] PageFlags& flags) {
        bool enabled = any(flags & PageFlags::WriteTrackingEnabled);
        if (enabled) {
            entry &= ~AP_FLAGS_MASK;
            entry |= static_cast<u64>(ApFlags::UserReadWriteKernelReadWrite);
            res = true;
        }
    });

    return res;
}

void PageTable::ResumeWriteTracking(Range<vaddr_t> range) {
    ModifyRange(range, []([[maybe_unused]] Range<vaddr_t> range, u64& entry,
                          [[maybe_unused]] horizon::kernel::MemoryState& state,
                          PageFlags& flags) {
        if (any(flags & PageFlags::WriteTrackingEnabled)) {
            entry &= ~AP_FLAGS_MASK;
            entry |= static_cast<u64>(ApFlags::UserReadKernelRead);
        }
    });
}

paddr_t PageTable::UnmapAddr(vaddr_t va) const {
    const auto region = QueryRegion(va);
    if (region.state.type == horizon::kernel::MemoryType::Free)
        return 0x0;
    // ASSERT_DEBUG(region.state.type != horizon::kernel::MemoryType::Free,
    //              Hypervisor, "Failed to unmap va 0x{:08x}", va);

    return region.UnmapAddr(va);
}

void PageTable::MapLevel(PageTableLevel& level, vaddr_t va, paddr_t pa,
                         usize size, const horizon::kernel::MemoryState state,
                         ApFlags ap_flags) {
    vaddr_t end_va = va + size;
    do {
        MapLevelNext(
            level, va, pa,
            std::min(align(va + 1, level.GetBlockSize()) - va, end_va - va),
            state, ap_flags);

        vaddr_t old_va = va;
        va = align_down(va + level.GetBlockSize(), level.GetBlockSize());
        pa += va - old_va;
    } while (va < end_va);
}

void PageTable::MapLevelNext(PageTableLevel& level, vaddr_t va, paddr_t pa,
                             usize size,
                             const horizon::kernel::MemoryState state,
                             ApFlags ap_flags) {
    // LOG_DEBUG(Hypervisor,
    //           "Level: {}, va: 0x{:08x}, pa: 0x{:08x}, size: 0x{:08x}",
    //           level.GetLevel(), va, pa, size);

    u32 index = level.VaToIndex(va);
    // TODO: uncomment
    if (/*size == level.GetBlockSize()*/ level.GetLevel() == 2) {
        level.GetEntry(index) = pa | PTE_BLOCK(level.GetLevel()) | PTE_AF |
                                PTE_INNER_SHEREABLE |
                                static_cast<u64>(ap_flags);
        level.GetLevelState(index) = state;
    } else {
        MapLevel(level.GetNext(allocator, index), va, pa, size, state,
                 ap_flags);
    }
}

void PageTable::IterateRange(
    Range<vaddr_t> range,
    std::function<void(Range<vaddr_t>, u64, const horizon::kernel::MemoryState&,
                       PageFlags)>
        callback) const {
    for (u64 page = range.GetBegin() / GUEST_PAGE_SIZE;
         page < range.GetEnd() / GUEST_PAGE_SIZE; ++page) {
        u32 index = top_level.VaToIndex(page * GUEST_PAGE_SIZE);
        auto* level = &top_level;
        u64 entry = top_level.GetEntry(index);
        while ((entry & PTE_TYPE_MASK) != PTE_BLOCK(level->GetLevel())) {
            if ((entry & PTE_TYPE_MASK) != PTE_TABLE)
                break;

            level = level->GetNextNoNew(index);
            index = level->VaToIndex(page * GUEST_PAGE_SIZE);
            entry = level->GetEntry(index);
        }

        if ((entry & PTE_TYPE_MASK) != PTE_TABLE)
            continue;

        callback(
            Range<vaddr_t>::FromSize(page * GUEST_PAGE_SIZE, GUEST_PAGE_SIZE),
            level->GetEntry(index), level->GetLevelState(index),
            level->GetLevelFlags(index));
    }
}

// TODO: this should subdivide the table if necessary
void PageTable::ModifyRange(
    Range<vaddr_t> range,
    std::function<void(Range<vaddr_t>, u64&, horizon::kernel::MemoryState&,
                       PageFlags&)>
        callback) {
    for (u64 page = range.GetBegin() / GUEST_PAGE_SIZE;
         page < range.GetEnd() / GUEST_PAGE_SIZE; ++page) {
        u32 index = top_level.VaToIndex(page * GUEST_PAGE_SIZE);
        auto* level = &top_level;
        u64 entry = top_level.GetEntry(index);
        while ((entry & PTE_TYPE_MASK) != PTE_BLOCK(level->GetLevel())) {
            if ((entry & PTE_TYPE_MASK) != PTE_TABLE)
                break;

            level = level->GetNextNoNew(index);
            index = level->VaToIndex(page * GUEST_PAGE_SIZE);
            entry = level->GetEntry(index);
        }

        if ((entry & PTE_TYPE_MASK) != PTE_TABLE)
            continue;

        callback(
            Range<vaddr_t>::FromSize(page * GUEST_PAGE_SIZE, GUEST_PAGE_SIZE),
            level->GetEntry(index), level->GetLevelState(index),
            level->GetLevelFlags(index));
    }
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
