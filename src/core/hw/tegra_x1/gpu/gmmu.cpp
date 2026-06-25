#include "core/hw/tegra_x1/gpu/gmmu.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::gpu {

uptr GMmu::UnmapAddr(uptr gpu_addr) const {
    uptr base;
    const auto& as = FindAddrImpl(gpu_addr, base);
    ASSERT_DEBUG(as.ptr != 0x0, Gpu, "Address 0x{:08x} is not host mapped",
                 gpu_addr);

    return as.ptr + (gpu_addr - base);
}

uptr GMmu::CreateAddressSpace(ztd::Range<vaddr_t> range, uptr gpu_addr) {
    uptr ptr;
    if (range.getBegin() != 0x0) {
        ptr = mmu->UnmapAddr(range.getBegin());

        // Write tracking
        mmu->EnableWriteTracking(range);
    } else {
        ptr = reinterpret_cast<uptr>(malloc(range.getSize()));
    }

    AddressSpace as;
    as.ptr = ptr;
    as.size = range.getSize();

    if (gpu_addr == invalid<uptr>()) {
        gpu_addr = address_space_base;
        address_space_base += align(range.getSize(), GPU_PAGE_SIZE);
    }
    Map(gpu_addr, as);

    return gpu_addr;
}

} // namespace hydra::hw::tegra_x1::gpu
