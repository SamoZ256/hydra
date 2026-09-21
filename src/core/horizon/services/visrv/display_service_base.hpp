#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"

namespace hydra::horizon::services::visrv {

class DisplayServiceBase : public IService {
  protected:
    result_t createStrayLayerImpl(
        System& system, kernel::Process* process, u32 flags, u64 display_id,
        u64* out_layer_id, u64* out_native_window_size,
        std::optional<ztd::io::MemoryStream> out_parcel_stream);
    result_t setLayerVisibilityImpl(u64 layer_id, bool visible);
};

} // namespace hydra::horizon::services::visrv
