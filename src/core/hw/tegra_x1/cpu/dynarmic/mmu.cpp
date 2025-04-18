#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"

#include "core/hw/tegra_x1/cpu/dynarmic/memory.hpp"

namespace Hydra::HW::TegraX1::CPU::Dynarmic {

MMU::MMU() {}

MMU::~MMU() {}

MemoryBase* MMU::AllocateMemory(usize size) {
    size = align(size, PAGE_SIZE);
    auto memory = new Memory(size);

    return memory;
}

void MMU::FreeMemory(MemoryBase* memory) { delete memory; }

uptr MMU::GetMemoryPtr(MemoryBase* memory) const {
    return static_cast<Memory*>(memory)->GetPtr();
}

void MMU::Map(vaddr_t va, usize size, MemoryBase* memory,
              const Horizon::MemoryState state) {
    ASSERT_ALIGNMENT(size, PAGE_SIZE, Dynarmic, "size");

    auto memory_ptr = static_cast<Memory*>(memory)->GetPtr();

    u64 va_page = va / PAGE_SIZE;
    u64 size_page = size / PAGE_SIZE;
    u64 va_page_end = va_page + size_page;
    for (u64 page = va_page; page < va_page_end; ++page) {
        auto page_ptr = memory_ptr + ((page - va_page) * PAGE_SIZE);
        pages[page] = page_ptr;
        states[page] = state;
    }
}

void MMU::Map(vaddr_t dst_va, vaddr_t src_va, usize size) {
    ASSERT_ALIGNMENT(size, PAGE_SIZE, Dynarmic, "size");

    auto src_page = src_va / PAGE_SIZE;
    auto dst_page = dst_va / PAGE_SIZE;
    auto size_page = size / PAGE_SIZE;
    for (u64 i = 0; i < size_page; i++) {
        pages[dst_page + i] = pages[src_page + i];
        states[dst_page + i] = states[src_page + i];
    }
}

void MMU::Unmap(vaddr_t va, usize size) {
    ASSERT_ALIGNMENT(size, PAGE_SIZE, Dynarmic, "size");

    auto va_page = va / PAGE_SIZE;
    auto size_page = size / PAGE_SIZE;
    auto va_page_end = va_page + size_page;
    for (u64 page = va_page; page < va_page_end; ++page) {
        pages[page] = 0x0;
        states[page] = {.type = Horizon::MemoryType::Free};
    }
}

void MMU::ResizeHeap(MemoryBase* heap_mem, vaddr_t va, usize size) {
    auto mem_impl = static_cast<Memory*>(heap_mem);

    mem_impl->Resize(size);

    auto memory_ptr = mem_impl->GetPtr();

    u64 va_page = va / PAGE_SIZE;
    u64 size_page = size / PAGE_SIZE;
    u64 va_page_end = va_page + size_page;
    for (u64 page = va_page; page < va_page_end; ++page) {
        auto page_ptr = memory_ptr + ((page - va_page) * PAGE_SIZE);
        pages[page] = page_ptr;
        states[page] = states[va_page];
    }
}

uptr MMU::UnmapAddr(vaddr_t va) const {
    auto page = va / PAGE_SIZE;
    auto page_offset = va % PAGE_SIZE;

    // HACK
    if (page >= sizeof_array(pages)) {
        LOG_WARNING(Dynarmic, "Failed to unmap va 0x{:08x}", va);
        static u64 zero = 0;
        return reinterpret_cast<uptr>(&zero);
    }
    // ASSERT_DEBUG(page < sizeof_array(pages), Dynarmic,
    //              "Addres out of range: 0x{:08x}", va);

    if (pages[page] == 0x0) {
        // TODO: error
        LOG_WARNING(Dynarmic, "Failed to unmap va 0x{:08x}", va);
        // HACK
        static u64 zero = 0;
        return reinterpret_cast<uptr>(&zero);
    }

    return pages[page] + page_offset;
}

MemoryRegion MMU::QueryRegion(vaddr_t va) const {
    return {
        .va = align_down(va, PAGE_SIZE),
        .size = PAGE_SIZE,
        .state = states[va / PAGE_SIZE],
    };
}

} // namespace Hydra::HW::TegraX1::CPU::Dynarmic
