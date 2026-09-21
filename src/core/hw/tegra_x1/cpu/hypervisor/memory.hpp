#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class Memory : public IMemory {
  public:
    explicit Memory(u64 size) : IMemory(size) { allocate(); }
    ~Memory() override { free(); }

    uptr getPtr() const override { return ptr; }

  protected:
    void resizeImpl() override {
        free();
        allocate();
    }

  private:
    uptr ptr;

    // Helpers
    u64 getSizeAligned() const { return align(getSize(), APPLE_PAGE_SIZE); }

    void allocate() {
        const auto size = getSizeAligned();
        ptr = allocateVmMemory(size);

        // Map
        // TODO: if AllocateVmMemory passes a pointer greater than 0x8000000000, this will fail
        HV_ASSERT_SUCCESS(
            hv_vm_map(reinterpret_cast<void*>(ptr), ptr, size,
                      HV_MEMORY_READ | HV_MEMORY_WRITE | HV_MEMORY_EXEC));
    }

    void free() {
        // Unmap
        HV_ASSERT_SUCCESS(
            hv_vm_unmap(ptr, align(getSizeAligned(), APPLE_PAGE_SIZE)));

        freeVmMemory(ptr, getSizeAligned());
    }
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
