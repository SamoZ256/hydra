#include "core/horizon/services/visrv/application_display_service.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"
#include "core/horizon/services/visrv/manager_display_service.hpp"
#include "core/horizon/services/visrv/system_display_service.hpp"

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
    IApplicationDisplayService, 100, GetRelayService, 101,
    GetSystemDisplayService, 102, GetManagerDisplayService, 103,
    GetIndirectDisplayTransactionService, 1000, ListDisplays, 1010, OpenDisplay,
    1020, CloseDisplay, 1102, GetDisplayResolution, 2020, OpenLayer, 2021,
    CloseLayer, 2030, CreateStrayLayer, 2031, DestroyStrayLayer, 2101,
    SetLayerScalingMode, 2102, ConvertScalingMode, 5202, GetDisplayVsyncEvent)

result_t IApplicationDisplayService::GetRelayService(RequestContext* ctx) {
    LOG_WARN(Services, "GetRelayService is not implemented properly");

    // TODO: this should wrap dispdrv in a custom class

    const auto name = "dispdrv"_u64;
    auto client_port = OS_INSTANCE.GetServiceManager().GetPort(name);
    if (!client_port) {
        LOG_WARN(Services, "Unknown service name \"{}\"", u64_to_str(name));
        return MAKE_RESULT(Svc, kernel::Error::NotFound); // TODO: module
    }

    // TODO: should this work with domains?
    ASSERT_DEBUG(!IsDomain(), Services,
                 "GetRelayService cannot be a domain service");
    auto client_session = client_port->Connect();
    const auto handle = ctx->process->AddHandle(client_session);
    ctx->writers.move_handles_writer.Write(handle);

    return RESULT_SUCCESS;
}

result_t
IApplicationDisplayService::GetSystemDisplayService(RequestContext* ctx) {
    AddService(*ctx, new ISystemDisplayService());
    return RESULT_SUCCESS;
}

result_t
IApplicationDisplayService::GetManagerDisplayService(RequestContext* ctx) {
    AddService(*ctx, new IManagerDisplayService());
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetIndirectDisplayTransactionService(
    RequestContext* ctx) {
    LOG_WARN(
        Services,
        "GetIndirectDisplayTransactionService is not implemented properly");

    // HACK
    return GetRelayService(ctx);
}

result_t IApplicationDisplayService::ListDisplays(
    u64* out_count, OutBuffer<BufferAttr::MapAlias> out_display_infos_buffer) {
    const auto res = OS_INSTANCE.GetDisplayResolution();
    out_display_infos_buffer.writer->Write<DisplayInfo>({
        .name = "Default",
        .has_layer_limit = true,
        .layer_count_max = 1,
        .layer_width_pixel_count_max = res.x(),
        .layer_height_pixel_count_max = res.y(),
    });
    *out_count = 1;
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::OpenDisplay(DisplayName display_name,
                                                 u64* out_display_id) {
    auto display_id =
        OS_INSTANCE.GetDisplayDriver().GetDisplayIDFromName(display_name.name);
    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(display_id);
    display.Open();

    *out_display_id = display_id;
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::CloseDisplay(u64 display_id) {
    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(
        static_cast<handle_id_t>(display_id));
    display.Close();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetDisplayResolution(u64 display_id,
                                                          i64* out_width,
                                                          i64* out_height) {
    LOG_FUNC_STUBBED(Services);

    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(
        static_cast<handle_id_t>(display_id));

    const auto res = OS_INSTANCE.GetDisplayResolution();
    *out_width = res.x();
    *out_height = res.y();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::OpenLayer(
    DisplayName display_name, u64 layer_id, u64 aruid,
    u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> parcel_buffer) {
    // TODO: what's the display for?
    // auto& display =
    // OS_INSTANCE.GetDisplayDriver().GetDisplayByName(display_name.name);

    auto& layer =
        OS_INSTANCE.GetDisplayDriver().GetLayer(static_cast<u32>(layer_id));
    layer.Open();

    // Parcel
    hosbinder::ParcelWriter parcel_writer(*parcel_buffer.writer);
    parcel_writer.WriteObject(layer.GetBinderID(), "dispdrv"_u64);
    parcel_writer.Finish();

    *out_native_window_size = parcel_writer.GetWrittenSize();

    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::CloseLayer(u64 layer_id) {
    OS_INSTANCE.GetDisplayDriver().DestroyLayer(static_cast<u32>(layer_id));
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::CreateStrayLayer(
    kernel::Process* process, aligned<u32, 8> flags, u64 display_id,
    u64* out_layer_id, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> out_parcel_buffer) {
    return CreateStrayLayerImpl(process, flags, display_id, out_layer_id,
                                out_native_window_size,
                                *out_parcel_buffer.writer);
}

result_t IApplicationDisplayService::DestroyStrayLayer(u64 layer_id) {
    // TODO: how is this different from CloseLayer?
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    CloseLayer(layer_id);
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::ConvertScalingMode() {
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetDisplayVsyncEvent(
    kernel::Process* process, u64 display_id,
    OutHandle<HandleAttr::Move> out_handle) {
    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(
        static_cast<handle_id_t>(display_id));

    out_handle = process->AddHandle(display.GetVSyncEvent());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
