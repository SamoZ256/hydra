#pragma once

#include <malloc/malloc.h>

#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

static inline uptr get_heap_base() {
    /*
    std::vector<uptr> ptrs;

    // Make several posix_memalign allocations
    for (int i = 0; i < 256; i++) {
        uptr ptr;
        if (posix_memalign(reinterpret_cast<void**>(&ptr), 0x1000, 0x1000) == 0)
            ptrs.push_back(ptr);
    }

    uptr lowest = ptrs[0];
    for (const auto ptr : ptrs) {
        if (ptr < lowest)
            lowest = ptr;
    }

    // Clean up
    for (const auto ptr : ptrs)
        free(reinterpret_cast<void*>(ptr));

    return lowest & ~0x0fffffffull;
    */

    // HACK
    return 0x100000000ull;
}

} // namespace

class PAMapper {
  public:
    PAMapper() {
        // Get physical memory start
        physical_mem_start = get_heap_base();
        LOG_DEBUG(Cpu, "Physical memory start: 0x{:08x}", physical_mem_start);
    }

    paddr_t GetPA(uptr ptr) const {
        ASSERT(ptr >= physical_mem_start, Hypervisor,
               "Memory address (0x{:08x}) must be greater or equal to physical "
               "memory "
               "start (0x{:08x})",
               ptr, physical_mem_start);
        return ptr - physical_mem_start;
    }
    uptr GetPtr(paddr_t pa) const { return physical_mem_start + pa; }

  private:
    uptr physical_mem_start;
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
