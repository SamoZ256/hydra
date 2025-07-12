#include "core/hw/tegra_x1/gpu/gpu_mmu.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::gpu {

uptr GpuMmu::UnmapAddr(uptr gpu_addr) {
    uptr base;
    auto as = FindAddrImpl(gpu_addr, base);
    ASSERT_DEBUG(as.ptr != 0x0, Gpu, "Address 0x{:08x} is not host mapped",
                 gpu_addr);

    return as.ptr + (gpu_addr - base);
}

} // namespace hydra::hw::tegra_x1::gpu
