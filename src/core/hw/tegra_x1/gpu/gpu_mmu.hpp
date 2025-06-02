#pragma once

#include "core/hw/generic_mmu.hpp"

namespace hydra::hw::tegra_x1::cpu {
class MMUBase;
}

namespace hydra::hw::tegra_x1::gpu {

enum class AsMemorySpace {
    Host,
    GuestCPU,
};

struct AddressSpace {
    AsMemorySpace space;
    uptr addr;
    usize size;
};

// TODO: free memory
class GpuMMU : public GenericMMU<GpuMMU, AddressSpace> {
  public:
    GpuMMU(cpu::MMUBase* mmu_) : mmu{mmu_} {}

    usize ImplGetSize(const AddressSpace& as) const { return as.size; }

    AddressSpace& UnmapAddrToAddressSpace(uptr gpu_addr) {
        usize base;
        auto addr_space = FindAddrImplRef(gpu_addr, base);
        ASSERT_DEBUG(addr_space, GPU,
                     "Address space not found for GPU address 0x{:x}",
                     gpu_addr);

        return *addr_space;
    }

    uptr UnmapAddr(uptr gpu_addr);

    void MapImpl(uptr base, AddressSpace as) {}
    void UnmapImpl(uptr base, AddressSpace as) {}

  private:
    cpu::MMUBase* mmu;
};

} // namespace hydra::hw::tegra_x1::gpu
