#include "horizon/services/nvdrv/ioctl/nvhost_as_gpu.hpp"

#include "hw/tegra_x1/gpu/gpu.hpp"

namespace Hydra::Horizon::Services::NvDrv::Ioctl {

DEFINE_IOCTL_TABLE(NvHostAsGpu,
                   DEFINE_IOCTL_TABLE_ENTRY(0x41, 0x02, AllocSpace, 0x06,
                                            MapBufferEx, 0x09, AllocASEx))

void NvHostAsGpu::AllocSpace(AllocSpaceData& data, NvResult& result) {
    uptr gpu_addr = invalid<uptr>();
    if (any(data.flags & AllocSpaceFlags::FixedOffset))
        gpu_addr = data.align_or_offset;

    data.offset =
        HW::TegraX1::GPU::GPU::GetInstance().AllocatePrivateAddressSpace(
            static_cast<usize>(data.pages) * static_cast<usize>(data.page_size),
            gpu_addr);
}

void NvHostAsGpu::MapBufferEx(MapBufferExData& data, NvResult& result) {
    auto& gpu = HW::TegraX1::GPU::GPU::GetInstance();

    if (any(data.flags & MapBufferFlags::Modify)) {
        LOG_NOT_IMPLEMENTED(HorizonServices,
                            "Address space modifying (address: 0x{:08x})",
                            data.offset.Get());
        return;
    }

    const auto& map = gpu.GetMap(data.nvmap_handle_id);

    usize size = data.mapping_size;
    if (size == 0x0)
        size = map.size; // TODO: correct?

    gpu_vaddr addr = invalid<uptr>();
    if (any(data.flags & MapBufferFlags::FixedOffset))
        addr = data.offset;

    data.offset =
        gpu.MapBufferToAddressSpace(map.addr + data.buffer_offset, size, addr);
    // LOG_DEBUG(HorizonServices, "OFFSET: 0x{:08x}", data.offset.Get());
}

void NvHostAsGpu::AllocASEx(AllocASExData& data, NvResult& result) {
    LOG_DEBUG(HorizonServices,
              "Start: 0x{:08x}, end: 0x{:08x}, split: 0x{:08x}",
              data.va_range_start.Get(), data.va_range_end.Get(),
              data.va_range_split.Get());

    // TODO: why does nouveau pass 0x0 for all of these?

    // TODO: what is split for?
    HW::TegraX1::GPU::GPU::GetInstance().AllocatePrivateAddressSpace(
        data.va_range_end - data.va_range_start, data.va_range_start);
}

} // namespace Hydra::Horizon::Services::NvDrv::Ioctl
