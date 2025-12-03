#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"

#include "core/hw/tegra_x1/cpu/dynarmic/memory.hpp"

// TODO: absolute offset page table

namespace hydra::hw::tegra_x1::cpu::dynarmic {

void Mmu::Map(vaddr_t dst_va, uptr ptr, usize size,
              const horizon::kernel::MemoryState state) {
    ASSERT_ALIGNMENT(size, GUEST_PAGE_SIZE, Dynarmic, "size");

    u64 va_page = dst_va / GUEST_PAGE_SIZE;
    u64 size_page = size / GUEST_PAGE_SIZE;
    u64 va_page_end = va_page + size_page;
    for (u64 page = va_page; page < va_page_end; ++page) {
        auto page_ptr = ptr + ((page - va_page) * GUEST_PAGE_SIZE);
        pages[page] = page_ptr;
        states[page] = state;
    }
}

void Mmu::Map(vaddr_t dst_va, vaddr_t src_va, usize size) {
    ASSERT_ALIGNMENT(size, GUEST_PAGE_SIZE, Dynarmic, "size");

    auto src_page = src_va / GUEST_PAGE_SIZE;
    auto dst_page = dst_va / GUEST_PAGE_SIZE;
    auto size_page = size / GUEST_PAGE_SIZE;
    for (u64 i = 0; i < size_page; i++) {
        pages[dst_page + i] = pages[src_page + i];
        states[dst_page + i] = states[src_page + i];
    }
}

void Mmu::Unmap(vaddr_t va, usize size) {
    ASSERT_ALIGNMENT(size, GUEST_PAGE_SIZE, Dynarmic, "size");

    auto va_page = va / GUEST_PAGE_SIZE;
    auto size_page = size / GUEST_PAGE_SIZE;
    auto va_page_end = va_page + size_page;
    for (u64 page = va_page; page < va_page_end; ++page) {
        pages[page] = 0x0;
        states[page] = {.type = horizon::kernel::MemoryType::Free};
    }
}

void Mmu::ResizeHeap(IMemory* heap_mem, vaddr_t va, usize size) {
    auto mem_impl = static_cast<Memory*>(heap_mem);

    mem_impl->Resize(size);

    auto memory_ptr = mem_impl->GetPtr();

    u64 va_page = va / GUEST_PAGE_SIZE;
    u64 size_page = size / GUEST_PAGE_SIZE;
    u64 va_page_end = va_page + size_page;
    for (u64 page = va_page; page < va_page_end; ++page) {
        auto page_ptr = memory_ptr + ((page - va_page) * GUEST_PAGE_SIZE);
        pages[page] = page_ptr;
        states[page] = states[va_page];
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
    return {
        .va = align_down(va, GUEST_PAGE_SIZE),
        .size = GUEST_PAGE_SIZE,
        .state = states[va / GUEST_PAGE_SIZE],
    };
}

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
