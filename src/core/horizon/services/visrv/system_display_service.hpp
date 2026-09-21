#pragma once

#include "core/horizon/services/visrv/display_service_base.hpp"

namespace hydra::horizon::services::visrv {

class ISystemDisplayService : public DisplayServiceBase {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getZOrderCountMax(u64 display_id, i64* out_count);
    result_t setLayerPosition(System* system, f32 x, f32 y, u64 layer_id);
    result_t setLayerSize(System* system, u64 layer_id, i64 width, i64 height);
    result_t setLayerZ(System* system, u64 layer_id, i64 z);
    result_t
    createStrayLayer(System* system, kernel::Process* process,
                     Aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
                     u64* out_native_window_size,
                     OutBuffer<BufferAttr::MapAlias> out_parcel_buffer);
    result_t setLayerVisibility(u64 layer_id, bool visible);
    result_t getDisplayMode(System* system, u64 display_id, u32* out_width,
                            u32* out_height, float* out_refresh_rate,
                            i32* out_unknown);
};

} // namespace hydra::horizon::services::visrv
