#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"

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

    auto& next = next_levels[index];
    if (!next) {
        next = new PageTableLevel(level + 1, allocator.GetNextPage(),
                                  base_va + index * GetBlockSize());
        WriteEntry(index, next->page.pa | PTE_TABLE);
    }

    return *next;
}

PageTable::PageTable(paddr_t base_pa)
    : allocator(base_pa, 1024), top_level(0, allocator.GetNextPage(), 0x0) {}

PageTable::~PageTable() = default;

void PageTable::Map(vaddr_t va, paddr_t pa, usize size,
                    const horizon::kernel::MemoryState state, ApFlags flags) {
    LOG_DEBUG(Hypervisor, "va: 0x{:08x}, pa: 0x{:08x}, size: 0x{:08x}", va, pa,
              size);

    ASSERT_ALIGNMENT(va, GUEST_PAGE_SIZE, Hypervisor, "va");
    ASSERT_ALIGNMENT(pa, GUEST_PAGE_SIZE, Hypervisor, "pa");
    ASSERT_ALIGNMENT(size, GUEST_PAGE_SIZE, Hypervisor, "size");

    MapLevel(top_level, va, pa, size, state, flags);
}

void PageTable::Unmap(vaddr_t va, usize size) {
    LOG_FUNC_NOT_IMPLEMENTED(Hypervisor);
}

// TODO: find out if there is a cheaper way
PageRegion PageTable::QueryRegion(vaddr_t va) const {
    u32 index = top_level.VaToIndex(va);
    auto* level = &top_level;
    u64 entry = top_level.ReadEntry(index);
    while ((entry & PTE_TYPE_MASK) != PTE_BLOCK(level->GetLevel())) {
        if ((entry & PTE_TYPE_MASK) != PTE_TABLE) {
            PageRegion region;
            region.va = va & ~(level->GetBlockSize() - 1);
            region.pa = 0x0;
            region.size = level->GetBlockSize();
            region.state = {horizon::kernel::MemoryType::Free,
                            horizon::kernel::MemoryAttribute::None,
                            horizon::kernel::MemoryPermission::None};

            return region;
        }

        level = level->GetNextNoNew(index);
        index = level->VaToIndex(va);
        entry = level->ReadEntry(index);
    }

    PageRegion region;
    region.va = va & ~(level->GetBlockSize() - 1);
    region.pa = (entry & ENTRY_ADDR_MASK);
    region.size = level->GetBlockSize();
    region.state = level->GetLevelState(index);

    return region;
}

paddr_t PageTable::UnmapAddr(vaddr_t va) const {
    const auto region = QueryRegion(va);
    ASSERT(region.state.type != horizon::kernel::MemoryType::Free, Hypervisor,
           "Failed to unmap va 0x{:08x}", va);

    return region.UnmapAddr(va);
}

void PageTable::MapLevel(PageTableLevel& level, vaddr_t va, paddr_t pa,
                         usize size, const horizon::kernel::MemoryState state,
                         ApFlags flags) {
    vaddr_t end_va = va + size;
    do {
        MapLevelNext(
            level, va, pa,
            std::min(align(va + 1, level.GetBlockSize()) - va, end_va - va),
            state, flags);

        vaddr_t old_va = va;
        va = align_down(va + level.GetBlockSize(), level.GetBlockSize());
        pa += va - old_va;
    } while (va < end_va);
}

void PageTable::MapLevelNext(PageTableLevel& level, vaddr_t va, paddr_t pa,
                             usize size,
                             const horizon::kernel::MemoryState state,
                             ApFlags flags) {
    // LOG_DEBUG(Hypervisor,
    //           "Level: {}, va: 0x{:08x}, pa: 0x{:08x}, size: 0x{:08x}",
    //           level.GetLevel(), va, pa, size);

    u32 index = level.VaToIndex(va);
    // TODO: uncomment
    if (/*size == level.GetBlockSize()*/ level.GetLevel() == 2) {
        level.WriteEntry(index, pa | PTE_BLOCK(level.GetLevel()) | PTE_AF |
                                    PTE_INNER_SHEREABLE | (u64)flags);
        level.SetLevelState(index, state);
    } else {
        MapLevel(level.GetNext(allocator, index), va, pa, size, state, flags);
    }
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
