#include "core/hw/tegra_x1/gpu/gpu_mmu.hpp"

#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::hw::tegra_x1::gpu {

uptr GpuMMU::UnmapAddr(uptr gpu_addr) {
    usize base;
    auto as = FindAddrImpl(gpu_addr, base);
    ASSERT_DEBUG(as.addr != 0x0, GPU, "Address 0x{:08x} is not host mapped",
                 gpu_addr);

    uptr space_addr = as.addr + (gpu_addr - base);
    switch (as.space) {
    case AsMemorySpace::Host:
        return space_addr;
    case AsMemorySpace::GuestCPU:
        return mmu->UnmapAddr(space_addr);
    }
}

} // namespace hydra::hw::tegra_x1::gpu
