#include "core/horizon/services/visrv/manager_display_service.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(IManagerDisplayService, 2012, CreateStrayLayer,
                             6002, SetLayerVisibility)

result_t IManagerDisplayService::CreateStrayLayer(
    aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
    u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    hosbinder::ParcelWriter parcel_writer(*out_parcel_buffer.writer);
    auto result = CreateStrayLayerImpl(flags, display_id, out_layer_id,
                                       out_native_window_size, parcel_writer);

    parcel_writer.Finalize();
    return RESULT_SUCCESS;
}

result_t IManagerDisplayService::SetLayerVisibility(u64 layer_id,
                                                    bool visible) {
    return SetLayerVisibilityImpl(layer_id, visible);
}

} // namespace hydra::horizon::services::visrv
