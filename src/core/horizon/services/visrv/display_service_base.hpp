#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"

namespace hydra::horizon::services::visrv {

class DisplayServiceBase : public IService {
  protected:
    result_t CreateStrayLayerImpl(kernel::Process* process, u32 flags,
                                  u64 display_id, u64* out_layer_id,
                                  u64* out_native_window_size,
                                  const Writer& out_parcel_writer);
    result_t SetLayerVisibilityImpl(u64 layer_id, bool visible);
};

} // namespace hydra::horizon::services::visrv
