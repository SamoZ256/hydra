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
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  protected:
    // Commands
    result_t GetRelayService(RequestContext* ctx, System* system);
    result_t GetSystemDisplayService(RequestContext* ctx);
    result_t GetManagerDisplayService(RequestContext* ctx);
    result_t GetIndirectDisplayTransactionService(RequestContext* ctx,
                                                  System* system);
    result_t
    ListDisplays(System* system, u64* out_count,
                 OutBuffer<BufferAttr::MapAlias> out_display_infos_buffer);
    result_t OpenDisplay(System* system, DisplayName display_name,
                         u64* out_display_id);
    result_t CloseDisplay(System* system, u64 display_id);
    result_t GetDisplayResolution(System* system, u64 display_id,
                                  i64* out_width, i64* out_height);
    result_t OpenLayer(System* system, DisplayName display_name, u64 layer_id,
                       u64 aruid, u64* out_native_window_size,
                       OutBuffer<BufferAttr::MapAlias> parcel_buffer);
    result_t CloseLayer(System* system, u64 layer_id);
    result_t
    CreateStrayLayer(System* system, kernel::Process* process,
                     aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
                     u64* out_native_window_size,
                     OutBuffer<BufferAttr::MapAlias> out_parcel_buffer);
    result_t DestroyStrayLayer(System* system, u64 layer_id);
    STUB_REQUEST_COMMAND(SetLayerScalingMode);
    result_t ConvertScalingMode();
    result_t GetDisplayVsyncEvent(System* system, kernel::Process* process,
                                  u64 display_id,
                                  OutHandle<HandleAttr::Move> out_handle);
};

} // namespace hydra::horizon::services::visrv
