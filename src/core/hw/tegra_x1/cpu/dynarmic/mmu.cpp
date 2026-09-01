#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"

#include "core/hw/tegra_x1/cpu/dynarmic/memory.hpp"

// TODO: absolute offset page table

namespace hydra::hw::tegra_x1::cpu::dynarmic {

void Mmu::Map(vaddr_t dst_va, ztd::Range<uptr> range,
              const horizon::kernel::MemoryState state) {
    ASSERT_ALIGNMENT(range.getSize(), GUEST_PAGE_SIZE, Dynarmic, "size");

    u64 va_page = dst_va / GUEST_PAGE_SIZE;
    u64 size_page = range.getSize() / GUEST_PAGE_SIZE;
    u64 va_page_end = va_page + size_page;
    for (u64 page = va_page; page < va_page_end; ++page) {
        auto page_ptr = range.getBegin() + ((page - va_page) * GUEST_PAGE_SIZE);
        pages[page] = page_ptr;
        states[page] = state;
    }
}

void Mmu::Map(vaddr_t dst_va, ztd::Range<vaddr_t> range) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Dynarmic, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Dynarmic, "end");

    auto src_page = range.getBegin() / GUEST_PAGE_SIZE;
    auto dst_page = dst_va / GUEST_PAGE_SIZE;
    for (u64 i = 0; i < range.getSize() / GUEST_PAGE_SIZE; i++) {
        pages[dst_page + i] = pages[src_page + i];
        states[dst_page + i] = states[src_page + i];
    }
}

void Mmu::Unmap(ztd::Range<vaddr_t> range) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Dynarmic, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Dynarmic, "end");

    for (u64 page = range.getBegin() / GUEST_PAGE_SIZE;
         page < range.getEnd() / GUEST_PAGE_SIZE; ++page) {
        pages[page] = 0x0;
        states[page] = {.type = horizon::kernel::MemoryType::Free};
    }
}

// TODO: actually protect the memory
void Mmu::Protect(ztd::Range<vaddr_t> range,
                  horizon::kernel::MemoryPermission perm) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Dynarmic, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Dynarmic, "end");

    for (u64 page = range.getBegin() / GUEST_PAGE_SIZE;
         page < range.getEnd() / GUEST_PAGE_SIZE; ++page) {
        states[page].perm = perm;
    }
}

uptr Mmu::UnmapAddr(vaddr_t va) const {
    auto page = va / GUEST_PAGE_SIZE;
    auto page_offset = va % GUEST_PAGE_SIZE;

    if (page >= PAGE_COUNT || pages[page] == 0x0)
        return 0x0;
    // ASSERT_DEBUG(page < PAGE_COUNT && pages[page] != 0x0, Dynarmic,
    //              "Address out of range: 0x{:08x}", va);

    return pages[page] + page_offset;
}

MemoryRegion Mmu::QueryRegion(vaddr_t va) const {
    const auto page = va / GUEST_PAGE_SIZE;
    if (page >= PAGE_COUNT)
        return {.va = page * GUEST_PAGE_SIZE,
                .size = GUEST_PAGE_SIZE,
                .state = {.type = horizon::kernel::MemoryType::Free}};

    return {
        .va = page * GUEST_PAGE_SIZE,
        .size = GUEST_PAGE_SIZE,
        .state = states[page],
    };
}

void Mmu::SetMemoryAttribute(ztd::Range<vaddr_t> range,
                             horizon::kernel::MemoryAttribute mask,
                             horizon::kernel::MemoryAttribute value) {
    ASSERT_ALIGNMENT(range.getBegin(), GUEST_PAGE_SIZE, Dynarmic, "begin");
    ASSERT_ALIGNMENT(range.getEnd(), GUEST_PAGE_SIZE, Dynarmic, "end");

    for (u64 page = range.getBegin() / GUEST_PAGE_SIZE;
         page < range.getEnd() / GUEST_PAGE_SIZE; ++page) {
        auto& state = states[page];
        state.attr = (state.attr & ~mask) | (value & mask);
    }
}

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
