#include "core/horizon/services/visrv/application_display_service.hpp"

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/os.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/hosbinder/hos_binder_driver.hpp"
#include "core/horizon/services/hosbinder/parcel.hpp"
#include "core/horizon/services/visrv/manager_display_service.hpp"
#include "core/horizon/services/visrv/system_display_service.hpp"
#include "core/hw/bus.hpp"
#include "core/hw/display/display.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(IApplicationDisplayService, 100, GetRelayService,
                             101, GetSystemDisplayService, 102,
                             GetManagerDisplayService, 103,
                             GetIndirectDisplayTransactionService, 1010,
                             OpenDisplay, 1020, CloseDisplay, 2020, OpenLayer,
                             2021, CloseLayer, 2101, SetLayerScalingMode, 2102,
                             ConvertScalingMode, 5202, GetDisplayVsyncEvent)

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

result_t IApplicationDisplayService::OpenDisplay(u64* out_display_id) {
    u64 display_id = 0; // TODO: get based on the name
    KERNEL_INSTANCE.GetBus().GetDisplay(display_id)->Open();

    *out_display_id = display_id;
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::CloseDisplay(u64 display_id) {
    KERNEL_INSTANCE.GetBus().GetDisplay(display_id)->Close();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::OpenLayer(
    u64 display_name, u64 layer_id, u64 aruid, u64* out_native_window_size,
    OutBuffer<BufferAttr::MapAlias> parcel_buffer) {
    u64 display_id = 0; // TODO: get based on the name

    auto layer =
        KERNEL_INSTANCE.GetBus().GetDisplay(display_id)->GetLayer(layer_id);
    layer->Open();

    // Out
    // TODO: correct?
    *out_native_window_size =
        sizeof(hosbinder::ParcelHeader) + sizeof(ParcelData);

    // Parcel
    hosbinder::ParcelWriter parcel_writer(*parcel_buffer.writer);

    parcel_writer.Write<ParcelData>({
        .unknown0 = 0x2,
        .unknown1 = 0x0, // TODO
        .binder_id = layer->GetBinderId(),
        .unknown2 = {0x0},
        .str = str_to_u64("dispdrv"),
        .unknown3 = 0x0,
    });

    parcel_writer.Finalize();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::CloseLayer(u64 layer_id) {
    u64 display_id = 0; // TODO: get from layer ID

    KERNEL_INSTANCE.GetBus()
        .GetDisplay(display_id)
        ->GetLayer(layer_id)
        ->Close();
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::ConvertScalingMode() {
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    return RESULT_SUCCESS;
}

result_t IApplicationDisplayService::GetDisplayVsyncEvent(
    u64 display_id, OutHandle<HandleAttr::Move> out_handle) {
    out_handle =
        KERNEL_INSTANCE.GetBus().GetDisplay(display_id)->GetVSyncEvent().id;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
