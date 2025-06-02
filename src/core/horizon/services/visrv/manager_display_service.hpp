#pragma once

#include "core/horizon/services/visrv/display_service_base.hpp"

namespace hydra::horizon::services::visrv {

class IManagerDisplayService : public DisplayServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t
    CreateStrayLayer(aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
                     u64* out_native_window_size,
                     OutBuffer<BufferAttr::MapAlias> out_parcel_buffer);
    result_t SetLayerVisibility(u64 layer_id, bool visible);
};

} // namespace hydra::horizon::services::visrv
