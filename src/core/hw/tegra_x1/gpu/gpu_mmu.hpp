#pragma once

#include "core/hw/generic_mmu.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

struct AddressSpace {
    uptr ptr;
    usize size;
};

// TODO: free memory
class GpuMmu : public GenericMmu<GpuMmu, AddressSpace> {
  public:
    usize ImplGetSize(const AddressSpace& as) const { return as.size; }

    AddressSpace& UnmapAddrToAddressSpace(uptr gpu_addr) {
        uptr base;
        auto addr_space = FindAddrImplRef(gpu_addr, base);
        ASSERT_DEBUG(addr_space, Gpu,
                     "Address space not found for Gpu address 0x{:x}",
                     gpu_addr);

        return *addr_space;
    }

    uptr UnmapAddr(uptr gpu_addr);

    void MapImpl(uptr base, AddressSpace as) {}
    void UnmapImpl(uptr base, AddressSpace as) {}
};

} // namespace hydra::hw::tegra_x1::gpu
