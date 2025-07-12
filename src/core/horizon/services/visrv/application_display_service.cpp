#include "core/horizon/services/visrv/application_display_service.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"
#include "core/horizon/services/hosbinder/hos_binder_driver.hpp"
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

DEFINE_SERVICE_COMMAND_TABLE(IApplicationDisplayService, 100, GetRelayService,
                             101, GetSystemDisplayService, 102,
                             GetManagerDisplayService, 103,
                             GetIndirectDisplayTransactionService, 1000,
                             ListDisplays, 1010, OpenDisplay, 1020,
                             CloseDisplay, 1102, GetDisplayResolution, 2020,
                             OpenLayer, 2021, CloseLayer, 2101,
                             SetLayerScalingMode, 2102, ConvertScalingMode,
                             5202, GetDisplayVsyncEvent)

result_t
IApplicationDisplayService::GetRelayService(add_service_fn_t add_service) {
    add_service(new hosbinder::IHOSBinderDriver());
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetSystemDisplayService(
    add_service_fn_t add_service) {
    add_service(new ISystemDisplayService());
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetManagerDisplayService(
    add_service_fn_t add_service) {
    add_service(new IManagerDisplayService());
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetIndirectDisplayTransactionService(
    add_service_fn_t add_service) {
    // TODO: how is this different from GetRelayService?
    add_service(new hosbinder::IHOSBinderDriver());
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::ListDisplays(
    u64* out_count, OutBuffer<BufferAttr::MapAlias> out_display_infos_buffer) {
    // TODO: don't hardcode
    out_display_infos_buffer.writer->Write<DisplayInfo>({
        .name = "Default",
        .has_layer_limit = true,
        .layer_count_max = 1,
        .layer_width_pixel_count_max = 1920,
        .layer_height_pixel_count_max = 1080,
    });
    *out_count = 1;
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::OpenDisplay(u64* out_display_id) {
    // TODO: what display ID should be used?
    const auto display_id = 0;
    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(display_id);
    std::unique_lock display_lock(display.GetMutex());
    display.Open();

    *out_display_id = display_id;
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::CloseDisplay(u64 display_id) {
    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(display_id);
    std::unique_lock display_lock(display.GetMutex());
    display.Close();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetDisplayResolution(u64 display_id,
                                                          i64* out_width,
                                                          i64* out_height) {
    LOG_FUNC_STUBBED(Services);

    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(display_id);
    std::unique_lock display_lock(display.GetMutex());

    // HACK
    *out_width = 1920;
    *out_height = 1080;
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::OpenLayer(
    u64 display_name, u64 layer_id, u64 aruid, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> parcel_buffer) {
    u64 display_id = 0; // TODO: get based on the name
    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(display_id);
    std::unique_lock display_lock(display.GetMutex());

    auto& layer = display.GetLayer(layer_id);
    layer.Open();

    // Out
    // TODO: correct?
    *out_native_window_size =
        sizeof(hosbinder::ParcelHeader) + sizeof(ParcelData);

    // Parcel
    hosbinder::ParcelWriter parcel_writer(*parcel_buffer.writer);

    parcel_writer.Write<ParcelData>({
        .unknown0 = 0x2,
        .unknown1 = 0x0, // TODO
        .binder_id = layer.GetBinderID(),
        .unknown2 = {0x0},
        .str = str_to_u64("dispdrv"),
        .unknown3 = 0x0,
    });

    parcel_writer.Finalize();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::CloseLayer(u64 layer_id) {
    u64 display_id = 0; // TODO: get from layer ID

    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(display_id);
    std::unique_lock display_lock(display.GetMutex());

    auto& layer = display.GetLayer(layer_id);
    layer.Close();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::ConvertScalingMode() {
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetDisplayVsyncEvent(
    kernel::Process* process, u64 display_id,
    OutHandle<HandleAttr::Move> out_handle) {
    auto& display = OS_INSTANCE.GetDisplayDriver().GetDisplay(display_id);
    std::unique_lock display_lock(display.GetMutex());

    out_handle = process->AddHandle(display.GetVSyncEvent());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
