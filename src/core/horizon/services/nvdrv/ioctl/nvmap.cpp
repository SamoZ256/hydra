#include "core/horizon/services/nvdrv/ioctl/nvmap.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::nvdrv::ioctl {

DEFINE_IOCTL_TABLE(NvMap,
                   DEFINE_IOCTL_TABLE_ENTRY(NvMap, 0x01, 0x01, Create, 0x03,
                                            FromId, 0x04, Alloc, 0x05, Free,
                                            0x09, Param, 0x0e, GetId))

NvResult NvMap::Create(System* system, u32 size, Handle* out_handle) {
    *out_handle = system->GetGpu().CreateMap(size).GetRaw();
    return NvResult::Success;
}

NvResult NvMap::FromId(u32 id, Handle* out_handle) {
    // Handle and ID are the same
    *out_handle = id;
    return NvResult::Success;
}

// TODO: heap mask, kind
NvResult NvMap::Alloc(System* system, Handle handle, u32 heap_mask, u32 flags,
                      InOutSingle<u32> inout_alignment, Aligned<u8, 8> kind,
                      gpu_vaddr_t addr) {
    (void)heap_mask;
    (void)kind;

    // TODO: flags?
    system->GetGpu().AllocateMap(handle, addr, flags == 1);
    inout_alignment = hw::tegra_x1::gpu::GPU_PAGE_SIZE; // TODO: correct?
    return NvResult::Success;
}

NvResult NvMap::Free(System* system, Aligned<Handle, 8> handle,
                     gpu_vaddr_t* out_addr, u64* out_size, u32* out_flags) {
    auto map = system->GetGpu().GetMap(handle).value();
    system->GetGpu().FreeMap(handle);

    *out_addr = map->addr;
    *out_size = map->size;
    *out_flags = map->write ? 1 : 0; // TODO: correct?
    return NvResult::Success;
}

NvResult NvMap::Param(System* system, Handle handle, NvMapParamType type,
                      u32* out_value) {
    auto map = system->GetGpu().GetMap(handle).value();
    switch (type) {
    case NvMapParamType::Size:
        *out_value = static_cast<u32>(map->size);
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

NvResult NvMap::GetId(u32* out_id, Handle handle) {
    // Handle and ID are the same
    *out_id = handle.GetRaw();
    return NvResult::Success;
}

} // namespace hydra::horizon::services::nvdrv::ioctl
