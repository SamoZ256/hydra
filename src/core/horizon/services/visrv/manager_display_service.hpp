#pragma once

#include "core/horizon/services/visrv/display_service_base.hpp"

namespace hydra::horizon::services::visrv {

class IManagerDisplayService : public DisplayServiceBase {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t createManagedLayer(System* system, kernel::Process* process,
                                Aligned<u32, 8> flags, u64 display_id,
                                u64 aruid, u64* out_layer_id);
    result_t destroyManagedLayer(System* system, u64 layer_id);
    result_t
    createStrayLayer(System* system, kernel::Process* process,
                     Aligned<u32, 8> flags, u64 display_id, u64* out_layer_id,
                     u64* out_native_window_size,
                     OutBuffer<BufferAttr::MapAlias> out_parcel_buffer);
    result_t addToLayerStack(u32 stack, u64 layer_id);
    result_t setLayerVisibility(u64 layer_id, bool visible);
};

} // namespace hydra::horizon::services::visrv
