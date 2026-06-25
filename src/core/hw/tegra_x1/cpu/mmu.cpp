#include "core/hw/tegra_x1/cpu/mmu.hpp"

#include "core/system.hpp"

namespace hydra::hw::tegra_x1::cpu {

horizon::kernel::MemoryInfo IMmu::QueryMemory(vaddr_t va) const {
    horizon::kernel::MemoryInfo info;
    info.size = 0x0;

    // Resize to the left
    auto region = QueryRegion(va);
    do {
        // Resize
        info.addr = region.va;
        info.size = region.size;
        info.state = region.state;
        if (info.addr == 0x0)
            break;

        // Next
        region = QueryRegion(info.addr - 1);
    } while (region.state == info.state);

    // Resize to the right
    region = QueryRegion(info.addr + info.size);
    while (region.state == info.state) {
        // Resize
        info.size += region.size;

        // Next
        vaddr_t addr = info.addr + info.size;
        if (addr >= horizon::kernel::ADDRESS_SPACE.getEnd())
            break;

        region = QueryRegion(addr);
    }

    // HACK
    if (info.state.type == horizon::kernel::MemoryType::Free) {
        info.ipc_ref_count = 0;
        info.device_ref_count = 0;
    } else {
        info.ipc_ref_count = 1;
        info.device_ref_count = 1;
    }

    return info;
}

vaddr_t IMmu::FindFreeMemory(ztd::Range<vaddr_t> region, u64 size) const {
    size = align(size, GUEST_PAGE_SIZE);
    auto crnt_region = ztd::Range<vaddr_t>::fromSize(region.getBegin(), size);
    while (region.contains(crnt_region)) {
        const auto info = QueryMemory(crnt_region.getBegin());
        const auto mem_range = ztd::Range<vaddr_t>(
            std::max(info.addr, region.getBegin()), info.addr + info.size);
        if (info.state.type == horizon::kernel::MemoryType::Free &&
            mem_range.contains(crnt_region))
            return mem_range.getBegin();

        crnt_region += mem_range.getSize();
    }

    return 0x0;
}

bool IMmu::TrackWrite(ztd::Range<vaddr_t> range) {
    const auto aligned_range =
        ztd::Range<vaddr_t>(align_down(range.getBegin(), GUEST_PAGE_SIZE),
                       align(range.getEnd(), GUEST_PAGE_SIZE));
    if (!TrySuspendWriteTracking(aligned_range))
        return false;

    // Notify the GPU
    // TODO: what about non-contiguous regions?
    const auto ptr = UnmapAddr(aligned_range.getBegin());
    system.GetGpu().GetRenderer().InvalidateMemory(
        ztd::Range<uptr>::fromSize(ptr, aligned_range.getSize()));

    {
        std::scoped_lock lock(write_tracking_mutex);
        tracked_pages.push_back(aligned_range);
    }

    return true;
}

void IMmu::FlushTrackedPages() {
    std::scoped_lock lock(write_tracking_mutex);
    for (const auto& range : tracked_pages)
        ResumeWriteTracking(range);
    tracked_pages.clear();
}

} // namespace hydra::hw::tegra_x1::cpu
