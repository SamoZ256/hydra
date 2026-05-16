#include "core/horizon/services/visrv/system_display_service.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(ISystemDisplayService, 1202, GetZOrderCountMax,
                             2201, SetLayerPosition, 2203, SetLayerSize, 2205,
                             SetLayerZ, 2207, SetLayerVisibility, 2312,
                             CreateStrayLayer, 3200, GetDisplayMode)

result_t ISystemDisplayService::GetZOrderCountMax(u64 display_id,
                                                  i64* out_count) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "display ID: {}", display_id);

    // HACK
    *out_count = 15;
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::SetLayerPosition(System* system, f32 x, f32 y,
                                                 u64 layer_id) {
    system->GetOS()
        .GetDisplayDriver()
        .GetLayer(static_cast<u32>(layer_id))
        .SetPosition({x, y});
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::SetLayerSize(System* system, u64 layer_id,
                                             i64 width, i64 height) {
    system->GetOS()
        .GetDisplayDriver()
        .GetLayer(static_cast<u32>(layer_id))
        .SetSize({u32(width), u32(height)});
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::SetLayerZ(System* system, u64 layer_id, i64 z) {
    system->GetOS()
        .GetDisplayDriver()
        .GetLayer(static_cast<u32>(layer_id))
        .SetZ(z);
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::CreateStrayLayer(
    System* system, kernel::Process* process, aligned<u32, 8> flags,
    u64 display_id, u64* out_layer_id, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    return CreateStrayLayerImpl(*system, process, flags, display_id,
                                out_layer_id, out_native_window_size,
                                out_parcel_buffer.stream);
}

result_t ISystemDisplayService::SetLayerVisibility(u64 layer_id, bool visible) {
    return SetLayerVisibilityImpl(layer_id, visible);
}

result_t ISystemDisplayService::GetDisplayMode(System* system, u64 display_id,
                                               u32* out_width, u32* out_height,
                                               float* out_refresh_rate,
                                               i32* out_unknown) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "display ID: {}", display_id);

    const auto res = system->GetOS().GetDisplayResolution();
    *out_width = res.x();
    *out_height = res.y();
    *out_refresh_rate = 60.0f;
    *out_unknown = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
