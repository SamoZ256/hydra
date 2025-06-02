#include "core/horizon/services/hid/hid_server.hpp"

#include "core/horizon/services/hid/active_vibration_device_list.hpp"
#include "core/horizon/services/hid/applet_resource.hpp"

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(
    IHidServer, 0, CreateAppletResource, 1, ActivateDebugPad, 11,
    ActivateTouchScreen, 21, ActivateMouse, 31, ActivateKeyboard, 66,
    StartSixAxisSensor, 67, StopSixAxisSensor, 79, SetGyroscopeZeroDriftMode,
    100, SetSupportedNpadStyleSet, 101, GetSupportedNpadStyleSet, 102,
    SetSupportedNpadIdType, 103, ActivateNpad, 106,
    AcquireNpadStyleSetUpdateEventHandle, 109, ActivateNpadWithRevision, 120,
    SetNpadJoyHoldType, 122, SetNpadJoyAssignmentModeSingleByDefault, 124,
    SetNpadJoyAssignmentModeDual, 128, SetNpadHandheldActivationMode, 200,
    GetVibrationDeviceInfo, 201, SendVibrationValue, 203,
    CreateActiveVibrationDeviceList, 206, SendVibrationValues)

// TODO: autoclear event?
IHidServer::IHidServer() : npad_style_set_update_event(new kernel::Event()) {}

result_t IHidServer::CreateAppletResource(kernel::add_service_fn_t add_service,
                                          u64 aruid) {
    add_service(new IAppletResource());

    return RESULT_SUCCESS;
}

result_t IHidServer::GetSupportedNpadStyleSet(
    ::hydra::horizon::hid::NpadStyleSet* style_set) {
    // TODO: make this configurable?
    *style_set = ::hydra::horizon::hid::NpadStyleSet::Standard;

    return RESULT_SUCCESS;
}

result_t IHidServer::AcquireNpadStyleSetUpdateEventHandle(
    u32 id, u32 _pad, u64 aruid, u64 event_ptr,
    OutHandle<HandleAttr::Copy> out_handle) {
    // TODO: params
    out_handle = npad_style_set_update_event.id;

    return RESULT_SUCCESS;
}

result_t IHidServer::GetVibrationDeviceInfo(VibrationDeviceHandle handle,
                                            VibrationDeviceInfo* info) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *info = {
        .device_type = VibrationDeviceType::LinearResonantActuator,
        .position = VibrationDevicePosition::Left,
    };

    return RESULT_SUCCESS;
}

result_t IHidServer::CreateActiveVibrationDeviceList(
    kernel::add_service_fn_t add_service) {
    add_service(new IActiveVibrationDeviceList());

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
