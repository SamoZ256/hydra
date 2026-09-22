#include "core/horizon/services/visrv/manager_display_service.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(IManagerDisplayService, 2010, createManagedLayer,
                             2011, destroyManagedLayer, 2012, createStrayLayer,
                             6000, addToLayerStack, 6002, setLayerVisibility)

// TODO: flags, display ID
result_t IManagerDisplayService::createManagedLayer(System* system,
                                                    kernel::Process* process,
                                                    Aligned<u32, 8> flags,
                                                    u64 display_id, u64 aruid,
                                                    u64* out_layer_id) {
    (void)flags;
    (void)display_id;
    (void)aruid;

    const auto binder_handle =
        system->getOs().getDisplayDriver().createBinder();
    // TODO: what's the display for?
    // auto& display =
    // system->GetOS().GetDisplayDriver().GetDisplay(display_id);

    *out_layer_id = system->getOs()
                        .getDisplayDriver()
                        .createLayer(process, binder_handle)
                        .getRaw();
    return RESULT_SUCCESS;
}

result_t IManagerDisplayService::destroyManagedLayer(System* system,
                                                     u64 layer_id) {
    system->getOs().getDisplayDriver().destroyLayer(static_cast<u32>(layer_id));
    return RESULT_SUCCESS;
}

result_t IManagerDisplayService::createStrayLayer(
    System* system, kernel::Process* process, Aligned<u32, 8> flags,
    u64 display_id, u64* out_layer_id, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    return createStrayLayerImpl(*system, process, flags, display_id,
                                out_layer_id, out_native_window_size,
                                out_parcel_buffer.stream);
}

result_t IManagerDisplayService::addToLayerStack(u32 stack, u64 layer_id) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "stack: {}, layer ID: {}", stack,
                               layer_id);
    return RESULT_SUCCESS;
}

result_t IManagerDisplayService::setLayerVisibility(u64 layer_id,
                                                    bool visible) {
    return setLayerVisibilityImpl(layer_id, visible);
}

} // namespace hydra::horizon::services::visrv
