#include "core/hw/tegra_x1/cpu/mmu.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"

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
        if (addr >= horizon::kernel::ADDRESS_SPACE.GetEnd())
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

vaddr_t IMmu::FindFreeMemory(Range<vaddr_t> region, usize size) const {
    size = align(size, GUEST_PAGE_SIZE);
    auto crnt_region = Range<vaddr_t>::FromSize(region.GetBegin(), size);
    while (region.Contains(crnt_region)) {
        const auto info = QueryMemory(crnt_region.GetBegin());
        const auto mem_range = Range<vaddr_t>(
            std::max(info.addr, region.GetBegin()), info.addr + info.size);
        if (info.state.type == horizon::kernel::MemoryType::Free &&
            mem_range.Contains(crnt_region))
            return mem_range.GetBegin();

        crnt_region += mem_range.GetSize();
    }

    return 0x0;
}

bool IMmu::TrackWrite(Range<vaddr_t> range) {
    const auto aligned_range =
        Range<vaddr_t>(align_down(range.GetBegin(), GUEST_PAGE_SIZE),
                       align(range.GetEnd(), GUEST_PAGE_SIZE));
    if (!TrySuspendWriteTracking(aligned_range))
        return false;

    // Notify the GPU
    // TODO: what about non-contiguous regions?
    const auto ptr = UnmapAddr(aligned_range.GetBegin());
    RENDERER_INSTANCE.InvalidateMemory(
        Range<uptr>::FromSize(ptr, aligned_range.GetSize()));

    {
        std::lock_guard lock(write_tracking_mutex);
        tracked_pages.push_back(aligned_range);
    }

    return true;
}

void IMmu::FlushTrackedPages() {
    std::lock_guard lock(write_tracking_mutex);
    for (const auto& range : tracked_pages)
        ResumeWriteTracking(range);
    tracked_pages.clear();
}

} // namespace hydra::hw::tegra_x1::cpu
