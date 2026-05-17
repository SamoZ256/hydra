#include "core/horizon/services/visrv/manager_display_service.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(IManagerDisplayService, 2010, CreateManagedLayer,
                             2011, DestroyManagedLayer, 2012, CreateStrayLayer,
                             6000, AddToLayerStack, 6002, SetLayerVisibility)

// TODO: flags, display ID
result_t IManagerDisplayService::CreateManagedLayer(System* system,
                                                    kernel::Process* process,
                                                    aligned<u32, 8> flags,
                                                    u64 display_id, u64 aruid,
                                                    u64* out_layer_id) {
    (void)flags;
    (void)display_id;
    (void)aruid;

    u32 binder_id = system->GetOS().GetDisplayDriver().CreateBinder();
    // TODO: what's the display for?
    // auto& display =
    // system->GetOS().GetDisplayDriver().GetDisplay(display_id);

    *out_layer_id =
        system->GetOS().GetDisplayDriver().CreateLayer(process, binder_id);
    return RESULT_SUCCESS;
}

result_t IManagerDisplayService::DestroyManagedLayer(System* system,
                                                     u64 layer_id) {
    system->GetOS().GetDisplayDriver().DestroyLayer(static_cast<u32>(layer_id));
    return RESULT_SUCCESS;
}

result_t IManagerDisplayService::CreateStrayLayer(
    System* system, kernel::Process* process, aligned<u32, 8> flags,
    u64 display_id, u64* out_layer_id, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    return CreateStrayLayerImpl(*system, process, flags, display_id,
                                out_layer_id, out_native_window_size,
                                out_parcel_buffer.stream);
}

result_t IManagerDisplayService::AddToLayerStack(u32 stack, u64 layer_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "stack: {}, layer ID: {}", stack,
                               layer_id);
    return RESULT_SUCCESS;
}

result_t IManagerDisplayService::SetLayerVisibility(u64 layer_id,
                                                    bool visible) {
    return SetLayerVisibilityImpl(layer_id, visible);
}

} // namespace hydra::horizon::services::visrv
