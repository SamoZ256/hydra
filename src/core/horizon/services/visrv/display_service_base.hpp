#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"

namespace hydra::horizon::services::visrv {

struct ParcelData {
    u32 unknown0;
    u32 unknown1;
    u32 binder_id;
    u32 unknown2[3];
    u64 str;
    u64 unknown3;
};

class DisplayServiceBase : public ServiceBase {
  protected:
    result_t CreateStrayLayerImpl(u32 flags, u64 display_id, u64* out_layer_id,
                                  u64* out_native_window_size,
                                  hosbinder::ParcelWriter& out_parcel_writer);
    result_t SetLayerVisibilityImpl(u64 layer_id, bool visible);
};

} // namespace hydra::horizon::services::visrv
