#pragma once

#include "core/horizon/services/visrv/display_service_base.hpp"

namespace hydra::horizon::services::hosbinder {
class IHOSBinderDriver;
}

namespace hydra::horizon::services::visrv {

struct DisplayName {
    char name[0x40];
};

class IApplicationDisplayService : public DisplayServiceBase {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

    // Commands
    result_t getRelayService(RequestContext* ctx, System* system);
    result_t getSystemDisplayService(RequestContext* ctx);
    result_t getManagerDisplayService(RequestContext* ctx);
    result_t getIndirectDisplayTransactionService(RequestContext* ctx,
                                                  System* system);
    result_t
    listDisplays(System* system, u64* out_count,
                 OutBuffer<BufferAttr::MapAlias> out_display_infos_buffer);
    result_t openDisplay(System* system, DisplayName display_name,
                         u64* out_display_id);
    result_t closeDisplay(System* system, u64 display_id);
    result_t getDisplayResolution(System* system, u64 display_id,
                                  i64* out_width, i64* out_height);
    result_t openLayer(System* system, DisplayName display_name, u64 layer_id,
                       u64 aruid, u64* out_native_window_size,
                       OutBuffer<BufferAttr::MapAlias> parcel_buffer);
    result_t closeLayer(System* system, u64 layer_id);
    result_t
    createStrayLayer(System* system, kernel::Process* process,
                     Aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
                     u64* out_native_window_size,
                     OutBuffer<BufferAttr::MapAlias> out_parcel_buffer);
    result_t destroyStrayLayer(System* system, u64 layer_id);
    STUB_REQUEST_COMMAND(setLayerScalingMode);
    result_t convertScalingMode();
    result_t getDisplayVsyncEvent(System* system, kernel::Process* process,
                                  u64 display_id,
                                  OutHandle<HandleAttr::Move> out_handle);
};

} // namespace hydra::horizon::services::visrv
