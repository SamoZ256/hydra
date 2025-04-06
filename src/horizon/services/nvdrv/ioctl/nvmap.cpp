#include "horizon/services/nvdrv/ioctl/nvmap.hpp"

#include "hw/tegra_x1/gpu/gpu.hpp"

namespace Hydra::Horizon::Services::NvDrv::Ioctl {

DEFINE_IOCTL_TABLE(NvMap, DEFINE_IOCTL_TABLE_ENTRY(0x01, 0x01, Create, 0x03,
                                                   FromId, 0x04, Alloc, 0x05,
                                                   Free, 0x0E, GetId))

void NvMap::Create(CreateData& data, NvResult& result) {
    data.handle_id = HW::TegraX1::GPU::GPU::GetInstance().CreateMap(data.size);
}

void NvMap::FromId(FromIdData& data, NvResult& result) {
    data.handle_id =
        HW::TegraX1::GPU::GPU::GetInstance().GetMapHandleId(data.id);
}

void NvMap::Alloc(AllocData& data, NvResult& result) {
    HW::TegraX1::GPU::GPU::GetInstance().AllocateMap(data.handle_id, data.addr,
                                                     data.flags == 1);
    data.alignment = HW::TegraX1::GPU::PAGE_SIZE; // TODO: correct?
}

void NvMap::Free(FreeData& data, NvResult& result) {
    auto& gpu = HW::TegraX1::GPU::GPU::GetInstance();
    auto map = gpu.GetMap(data.handle_id);
    gpu.FreeMap(data.handle_id);

    data.addr = map.addr;
    data.size = map.size;
    data.flags = map.write ? 1 : 0; // TODO: correct?
}

void NvMap::GetId(GetIdData& data, NvResult& result) {
    data.id = HW::TegraX1::GPU::GPU::GetInstance().GetMapId(data.handle_id);
}

} // namespace Hydra::Horizon::Services::NvDrv::Ioctl
