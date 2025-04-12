#include "core/hw/tegra_x1/cpu/hypervisor/page_allocator.hpp"

namespace Hydra::HW::TegraX1::CPU::Hypervisor {

PageAllocator::PageAllocator(paddr base_pa_, usize page_count)
    : base_pa{base_pa_} {
    Allocate(page_count);
}

PageAllocator::~PageAllocator() {
    for (const auto allocation : allocations) {
        free(reinterpret_cast<void*>(allocation.ptr));
    }
}

Page PageAllocator::GetNextPage() {
    if (current_page_in_allocation >= allocations.back().page_count) {
        Allocate();
        current_page_in_allocation = 0;
    }

    Page page;
    page.ptr =
        allocations.back().ptr + current_page_in_allocation++ * PAGE_SIZE;
    page.pa = base_pa + current_page++ * PAGE_SIZE;

    return page;
}

void PageAllocator::Allocate(usize page_count) {
    page_count = align(page_count, PAGE_COUNT_ALIGNMENT);

    const usize size = page_count * PAGE_SIZE;
    uptr ptr = allocate_vm_memory(size);

    const paddr pa = base_pa + current_page * PAGE_SIZE;
    HV_ASSERT_SUCCESS(
        hv_vm_map(reinterpret_cast<void*>(ptr), pa, size, HV_MEMORY_READ));

    allocations.push_back({ptr, page_count});
}

} // namespace Hydra::HW::TegraX1::CPU::Hypervisor
