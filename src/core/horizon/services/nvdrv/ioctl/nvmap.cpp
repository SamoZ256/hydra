#include "core/horizon/services/nvdrv/ioctl/nvmap.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvMap,
                   DEFINE_IOCTL_TABLE_ENTRY(NvMap, 0x01, 0x01, Create, 0x03,
                                            FromId, 0x04, Alloc, 0x05, Free,
                                            0x09, Param, 0x0e, GetId))

NvResult NvMap::Create(u32 size, handle_id_t* out_handle_id) {
    *out_handle_id = GPU_INSTANCE.CreateMap(size);
    return NvResult::Success;
}

NvResult NvMap::FromId(u32 id, handle_id_t* out_handle_id) {
    // Handle and ID are the same
    *out_handle_id = id;
    return NvResult::Success;
}

NvResult NvMap::Alloc(handle_id_t handle_id, u32 heap_mask, u32 flags,
                      InOutSingle<u32> inout_alignment, aligned<u8, 8> kind,
                      gpu_vaddr_t addr) {
    // TODO: flags wtf
    // TODO: kind
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

NvResult NvMap::Param(handle_id_t handle_id, NvMapParamType type,
                      u32* out_value) {
    auto map = GPU_INSTANCE.GetMap(handle_id);
    switch (type) {
    case NvMapParamType::Size:
        *out_value = static_cast<u32>(map.size);
        break;
    case NvMapParamType::Alignment:
        *out_value = hw::tegra_x1::gpu::GPU_PAGE_SIZE; // TODO: correct?
        break;
    case NvMapParamType::Heap:
        *out_value = 0x40000000;
        break;
    case NvMapParamType::Kind:
        LOG_NOT_IMPLEMENTED(Services, "NvMapParamType::Kind");
        // TODO
        //*out_value = map.kind;
        *out_value = 0;
        break;
    case NvMapParamType::Compr:
        LOG_NOT_IMPLEMENTED(Services, "NvMapParamType::Compr");
        *out_value = 0;
        break;
    default: // Also includes Base
        // TODO: return NvInternalResult::InvalidInput?
        return NvResult::NotSupported;
    }
    return NvResult::Success;
}

NvResult NvMap::GetId(u32* out_id, handle_id_t handle_id) {
    // Handle and ID are the same
    *out_id = handle_id;
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
