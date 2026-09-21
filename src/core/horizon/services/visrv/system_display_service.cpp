#include "core/horizon/services/visrv/system_display_service.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(ISystemDisplayService, 1202, getZOrderCountMax,
                             2201, setLayerPosition, 2203, setLayerSize, 2205,
                             setLayerZ, 2207, setLayerVisibility, 2312,
                             createStrayLayer, 3200, getDisplayMode)

result_t ISystemDisplayService::getZOrderCountMax(u64 display_id,
                                                  i64* out_count) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "display ID: {}", display_id);

    // HACK
    *out_count = 15;
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::setLayerPosition(System* system, f32 x, f32 y,
                                                 u64 layer_id) {
    system->getOs()
        .getDisplayDriver()
        .getLayer(static_cast<u32>(layer_id))
        .setPosition({x, y});
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::setLayerSize(System* system, u64 layer_id,
                                             i64 width, i64 height) {
    system->getOs()
        .getDisplayDriver()
        .getLayer(static_cast<u32>(layer_id))
        .setSize({static_cast<u32>(width), static_cast<u32>(height)});
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::setLayerZ(System* system, u64 layer_id, i64 z) {
    system->getOs()
        .getDisplayDriver()
        .getLayer(static_cast<u32>(layer_id))
        .setZ(z);
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::createStrayLayer(
    System* system, kernel::Process* process, Aligned<u32, 8> flags,
    u64 display_id, u64* out_layer_id, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    return createStrayLayerImpl(*system, process, flags, display_id,
                                out_layer_id, out_native_window_size,
                                out_parcel_buffer.stream);
}

result_t ISystemDisplayService::setLayerVisibility(u64 layer_id, bool visible) {
    return setLayerVisibilityImpl(layer_id, visible);
}

result_t ISystemDisplayService::getDisplayMode(System* system, u64 display_id,
                                               u32* out_width, u32* out_height,
                                               float* out_refresh_rate,
                                               i32* out_unknown) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "display ID: {}", display_id);

    const auto res = system->getOs().getDisplayResolution();
    *out_width = res.x();
    *out_height = res.y();
    *out_refresh_rate = 60.0f;
    *out_unknown = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
