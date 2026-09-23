#include "core/hw/tegra_x1/gpu/gmmu.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::hw::tegra_x1::gpu {

uptr GMmu::unmapAddr(uptr gpu_addr) const {
    uptr base;
    const auto& as = findAddrImpl(gpu_addr, base);
    ASSERT_DEBUG(as.ptr != 0x0, Gpu, "Address 0x{:08x} is not host mapped",
                 gpu_addr);

    return as.ptr + (gpu_addr - base);
}

uptr GMmu::createAddressSpace(ztd::Range<vaddr_t> range, uptr gpu_addr) {
    uptr ptr;
    if (range.getBegin() != 0x0) {
        ptr = mmu->unmapAddr(range.getBegin());

        // Write tracking
        mmu->enableWriteTracking(range);
    } else {
        // NOLINTNEXTLINE(cppcoreguidelines-no-malloc)
        ptr = reinterpret_cast<uptr>(malloc(range.getSize()));
    }

    AddressSpace as;
    as.ptr = ptr;
    as.size = range.getSize();

    if (gpu_addr == invalid<uptr>()) {
        gpu_addr = address_space_base;
        address_space_base += align(range.getSize(), GPU_PAGE_SIZE);
    }
    map(gpu_addr, as);

    return gpu_addr;
}

} // namespace hydra::hw::tegra_x1::gpu
