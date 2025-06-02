#pragma once

#include "core/horizon/services/visrv/display_service_base.hpp"

namespace hydra::horizon::services::hosbinder {
class IHOSBinderDriver;
}

namespace hydra::horizon::services::visrv {

class IApplicationDisplayService : public DisplayServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  protected:
    // Commands
    result_t GetRelayService(add_service_fn_t add_service);
    result_t GetSystemDisplayService(add_service_fn_t add_service);
    result_t GetManagerDisplayService(add_service_fn_t add_service);
    result_t GetIndirectDisplayTransactionService(add_service_fn_t add_service);
    result_t OpenDisplay(u64* out_display_id);
    result_t CloseDisplay(u64 display_id);
    result_t OpenLayer(u64 display_name, u64 layer_id, u64 aruid,
                       u64* out_native_window_size,
                       OutBuffer<BufferAttr::MapAlias> parcel_buffer);
    result_t CloseLayer(u64 layer_id);
    STUB_REQUEST_COMMAND(SetLayerScalingMode);
    result_t ConvertScalingMode();
    result_t GetDisplayVsyncEvent(u64 display_id,
                                  OutHandle<HandleAttr::Move> out_handle);

  private:
};

} // namespace hydra::horizon::services::visrv
