#include "core/horizon/services/nvdrv/ioctl/nvmap.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvMap, DEFINE_IOCTL_TABLE_ENTRY(NvMap, 0x01, 0x01, Create,
                                                   0x03, FromId, 0x04, Alloc,
                                                   0x05, Free, 0x0E, GetId))

NvResult NvMap::Create(u32 size, handle_id_t* out_handle_id) {
    *out_handle_id = GPU_INSTANCE.CreateMap(size);
    return NvResult::Success;
}

NvResult NvMap::FromId(u32 id, handle_id_t* out_handle_id) {
    *out_handle_id = GPU_INSTANCE.GetMapHandleId(id);
    return NvResult::Success;
}

NvResult NvMap::Alloc(handle_id_t handle_id, u32 heap_mask, u32 flags,
                      InOutSingle<u32> inout_alignment, aligned<u8, 8> kind,
                      gpu_vaddr_t addr) {
    // TODO: flags wtf
    GPU_INSTANCE.AllocateMap(handle_id, addr, flags == 1);
    inout_alignment = hw::tegra_x1::gpu::GPU_PAGE_SIZE; // TODO: correct?
    return NvResult::Success;
}

NvResult NvMap::Free(aligned<handle_id_t, 8> handle_id, gpu_vaddr_t* out_addr,
                     u64* out_size, u32* out_flags) {
    auto map = GPU_INSTANCE.GetMap(handle_id);
    GPU_INSTANCE.FreeMap(handle_id);

    *out_addr = map.addr;
    *out_size = map.size;
    *out_flags = map.write ? 1 : 0; // TODO: correct?
    return NvResult::Success;
}

NvResult NvMap::GetId(u32* out_id, handle_id_t handle_id) {
    *out_id = GPU_INSTANCE.GetMapId(handle_id);
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
