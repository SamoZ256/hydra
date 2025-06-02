#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

struct Page {
    uptr ptr;
    paddr_t pa;
};

struct Allocation {
    uptr ptr;
    usize page_count;
};

constexpr usize PAGE_COUNT_ALIGNMENT = 4;

class PageAllocator {
  public:
    PageAllocator(paddr_t base_pa_, usize page_count);
    ~PageAllocator();

    Page GetNextPage();

    // Getters
    paddr_t GetBase() const { return base_pa; }

  private:
    paddr_t base_pa;

    std::vector<Allocation> allocations;
    u32 current_page_in_allocation = 0;
    usize current_page = 0;

    void Allocate(usize page_count = PAGE_COUNT_ALIGNMENT);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
