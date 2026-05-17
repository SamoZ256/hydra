#pragma once

#include "core/horizon/services/visrv/display_service_base.hpp"

namespace hydra::horizon::services::visrv {

class ISystemDisplayService : public DisplayServiceBase {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t GetZOrderCountMax(u64 display_id, i64* out_count);
    result_t SetLayerPosition(System* system, f32 x, f32 y, u64 layer_id);
    result_t SetLayerSize(System* system, u64 layer_id, i64 width, i64 height);
    result_t SetLayerZ(System* system, u64 layer_id, i64 z);
    result_t
    CreateStrayLayer(System* system, kernel::Process* process,
                     aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
                     u64* out_native_window_size,
                     OutBuffer<BufferAttr::MapAlias> out_parcel_buffer);
    result_t SetLayerVisibility(u64 layer_id, bool visible);
    result_t GetDisplayMode(System* system, u64 display_id, u32* out_width,
                            u32* out_height, float* out_refresh_rate,
                            i32* out_unknown);
};

} // namespace hydra::horizon::services::visrv
