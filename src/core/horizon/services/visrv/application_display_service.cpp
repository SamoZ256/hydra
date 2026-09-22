#include "core/horizon/services/visrv/application_display_service.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"
#include "core/horizon/services/visrv/manager_display_service.hpp"
#include "core/horizon/services/visrv/system_display_service.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::visrv {

namespace {

struct DisplayInfo {
    char name[0x40];
    bool has_layer_limit;
    u8 _reserved_x41[0x7];
    u64 layer_count_max;
    u64 layer_width_pixel_count_max;
    u64 layer_height_pixel_count_max;
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(
    IApplicationDisplayService, 100, getRelayService, 101,
    getSystemDisplayService, 102, getManagerDisplayService, 103,
    getIndirectDisplayTransactionService, 1000, listDisplays, 1010, openDisplay,
    1020, closeDisplay, 1102, getDisplayResolution, 2020, openLayer, 2021,
    closeLayer, 2030, createStrayLayer, 2031, destroyStrayLayer, 2101,
    setLayerScalingMode, 2102, convertScalingMode, 5202, getDisplayVsyncEvent)

result_t IApplicationDisplayService::getRelayService(RequestContext* ctx,
                                                     System* system) {
    LOG_WARN(Services, "GetRelayService is not implemented properly");

    // TODO: this should wrap dispdrv in a custom class

    const auto name = "dispdrv"_u64;
    auto client_port = system->getOs().getServiceManager().getPort(name);
    if (client_port == nullptr) {
        LOG_WARN(Services, "Unknown service name \"{}\"", u64AsString(name));
        return MAKE_RESULT(Svc, kernel::Error::NotFound); // TODO: module
    }

    // TODO: should this work with domains?
    ASSERT_DEBUG(!isDomain(), Services,
                 "GetRelayService cannot be a domain service");
    auto client_session = client_port->connect();
    const auto handle = ctx->process->addHandle(client_session);
    ctx->streams.out_move_handles_stream.write(handle);

    return RESULT_SUCCESS;
}

result_t
IApplicationDisplayService::getSystemDisplayService(RequestContext* ctx) {
    addService(*ctx, new ISystemDisplayService());
    return RESULT_SUCCESS;
}

result_t
IApplicationDisplayService::getManagerDisplayService(RequestContext* ctx) {
    addService(*ctx, new IManagerDisplayService());
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::getIndirectDisplayTransactionService(
    RequestContext* ctx, System* system) {
    LOG_WARN(
        Services,
        "GetIndirectDisplayTransactionService is not implemented properly");

    // HACK
    return getRelayService(ctx, system);
}

result_t IApplicationDisplayService::listDisplays(
    System* system, u64* out_count,
    OutBuffer<BufferAttr::MapAlias> out_display_infos_buffer) {
    const auto res = system->getOs().getDisplayResolution();
    out_display_infos_buffer.stream->write<DisplayInfo>({
        .name = "Default",
        .has_layer_limit = true,
        .layer_count_max = 1,
        .layer_width_pixel_count_max = res.x(),
        .layer_height_pixel_count_max = res.y(),
    });
    *out_count = 1;
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::openDisplay(System* system,
                                                 DisplayName display_name,
                                                 u64* out_display_id) {
    const auto display_handle =
        system->getOs().getDisplayDriver().getDisplayIdFromName(
            display_name.name);
    auto& display =
        system->getOs().getDisplayDriver().getDisplay(display_handle);
    display.open();

    *out_display_id = display_handle.getRaw();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::closeDisplay(System* system,
                                                  u64 display_id) {
    auto& display = system->getOs().getDisplayDriver().getDisplay(
        static_cast<u32>(display_id));
    display.close();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::getDisplayResolution(System* system,
                                                          u64 display_id,
                                                          i64* out_width,
                                                          i64* out_height) {
    auto& display = system->getOs().getDisplayDriver().getDisplay(
        static_cast<u32>(display_id));
    (void)display;

    // TODO: use the display
    const auto res = system->getOs().getDisplayResolution();
    *out_width = res.x();
    *out_height = res.y();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::openLayer(
    System* system, DisplayName display_name, u64 layer_id, u64 aruid,
    u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> parcel_buffer) {
    (void)display_name;
    (void)aruid;

    // TODO: what's the display for?
    // auto& display =
    // system->GetOS().GetDisplayDriver().GetDisplayByName(display_name.name);

    auto& layer =
        system->getOs().getDisplayDriver().getLayer(static_cast<u32>(layer_id));
    layer.open();

    // Parcel
    hosbinder::ParcelWriter parcel_writer(parcel_buffer.stream.value());
    parcel_writer.writeObject(layer.getBinderHandle().getRaw(), "dispdrv"_u64);
    parcel_writer.finish();

    *out_native_window_size = parcel_writer.getWrittenSize();

    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::closeLayer(System* system, u64 layer_id) {
    system->getOs().getDisplayDriver().destroyLayer(static_cast<u32>(layer_id));
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::createStrayLayer(
    System* system, kernel::Process* process, Aligned<u32, 8> flags,
    u64 display_id, u64* out_layer_id, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    return createStrayLayerImpl(*system, process, flags, display_id,
                                out_layer_id, out_native_window_size,
                                out_parcel_buffer.stream);
}

result_t IApplicationDisplayService::destroyStrayLayer(System* system,
                                                       u64 layer_id) {
    // TODO: how is this different from CloseLayer?
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    closeLayer(system, layer_id);
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::convertScalingMode() {
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::getDisplayVsyncEvent(
    System* system, kernel::Process* process, u64 display_id,
    OutHandle<HandleAttr::Move> out_handle) {
    auto& display = system->getOs().getDisplayDriver().getDisplay(
        static_cast<u32>(display_id));

    out_handle = process->addHandle(display.getVSyncEvent());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
