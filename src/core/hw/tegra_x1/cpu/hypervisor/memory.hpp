#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/pa_mapper.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class Memory : public IMemory {
  public:
    Memory(const PAMapper& pa_mapper, usize size) : IMemory(size) {
        size = align(size, APPLE_PAGE_SIZE);
        ptr = allocate_vm_memory(size);

        // Guest physical memory = host virtual memory
        HV_ASSERT_SUCCESS(
            hv_vm_map(reinterpret_cast<void*>(ptr), pa_mapper.GetPA(ptr), size,
                      HV_MEMORY_READ | HV_MEMORY_WRITE | HV_MEMORY_EXEC));
    }

    ~Memory() override { free(reinterpret_cast<void*>(ptr)); }

    void Resize(usize new_size) {
        free(reinterpret_cast<void*>(ptr));
        ptr = reinterpret_cast<uptr>(
            allocate_vm_memory(align(new_size, APPLE_PAGE_SIZE)));
    }

    uptr GetPtr() const override { return ptr; }

  private:
    uptr ptr;
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
