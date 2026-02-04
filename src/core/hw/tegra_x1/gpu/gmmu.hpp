#pragma once

#include "core/hw/generic_mmu.hpp"
#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu {

struct AddressSpace {
    uptr ptr;
    usize size;
};

// TODO: free memory
class GMmu : public GenericMmu<GMmu, AddressSpace> {
  public:
    GMmu(cpu::IMmu* mmu_) : mmu{mmu_} {}

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

    void MapImpl([[maybe_unused]] uptr base, [[maybe_unused]] AddressSpace as) {
    }
    void UnmapImpl([[maybe_unused]] uptr base,
                   [[maybe_unused]] AddressSpace as) {}

    // Address space
    uptr CreateAddressSpace(Range<vaddr_t> range, uptr gpu_addr);

    uptr AllocatePrivateAddressSpace(usize size, uptr gpu_addr) {
        return CreateAddressSpace(Range<vaddr_t>::FromSize(0x0, size),
                                  gpu_addr);
    }

    uptr MapBufferToAddressSpace(Range<vaddr_t> range, uptr gpu_addr) {
        return CreateAddressSpace(range, gpu_addr);
    }

    // TODO
    /*
    void ModifyAddressSpace(uptr ptr, usize size, uptr gpu_addr) {
        auto& as = UnmapAddrToAddressSpace(gpu_addr);
        ASSERT_DEBUG(size == as.size, Gpu, "Size mismatch: {} != {}", size,
                     as.size)

        as.ptr = ptr;
    }
    */

  private:
    cpu::IMmu* mmu;

    // TODO: use a better way to allocate new memory
    uptr address_space_base{GPU_PAGE_SIZE};

  public:
    GETTER(mmu, GetMmu);
};

} // namespace hydra::hw::tegra_x1::gpu
