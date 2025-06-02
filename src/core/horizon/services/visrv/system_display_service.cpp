#include "core/horizon/services/visrv/system_display_service.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(ISystemDisplayService, 2207, SetLayerVisibility,
                             2312, CreateStrayLayer, 3200, GetDisplayMode)

result_t ISystemDisplayService::CreateStrayLayer(
    aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
    u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    hosbinder::ParcelWriter parcel_writer(*out_parcel_buffer.writer);
    auto result = CreateStrayLayerImpl(flags, display_id, out_layer_id,
                                       out_native_window_size, parcel_writer);

    parcel_writer.Finalize();
    return RESULT_SUCCESS;
}

result_t ISystemDisplayService::SetLayerVisibility(u64 layer_id, bool visible) {
    return SetLayerVisibility(layer_id, visible);
}

result_t ISystemDisplayService::GetDisplayMode(u64 display_id, u32* out_width,
                                               u32* out_height,
                                               float* out_refresh_rate,
                                               i32* out_unknown) {
    LOG_FUNC_STUBBED(Services);

    // TODO: get this from the display
    *out_width = 1280;
    *out_height = 720;
    *out_refresh_rate = 60.0f;
    *out_unknown = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
