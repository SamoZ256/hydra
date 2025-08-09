#include "core/horizon/services/hid/hid_server.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/hid/active_vibration_device_list.hpp"
#include "core/horizon/services/hid/applet_resource.hpp"

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(
    IHidServer, 0, CreateAppletResource, 1, ActivateDebugPad, 11,
    ActivateTouchScreen, 21, ActivateMouse, 31, ActivateKeyboard, 66,
    StartSixAxisSensor, 67, StopSixAxisSensor, 79, SetGyroscopeZeroDriftMode,
    100, SetSupportedNpadStyleSet, 101, GetSupportedNpadStyleSet, 102,
    SetSupportedNpadIdType, 103, ActivateNpad, 106,
    AcquireNpadStyleSetUpdateEventHandle, 108, GetPlayerLedPattern, 109,
    ActivateNpadWithRevision, 120, SetNpadJoyHoldType, 121, GetNpadJoyHoldType,
    122, SetNpadJoyAssignmentModeSingleByDefault, 124,
    SetNpadJoyAssignmentModeDual, 128, SetNpadHandheldActivationMode, 130,
    SwapNpadAssignment, 200, GetVibrationDeviceInfo, 201, SendVibrationValue,
    203, CreateActiveVibrationDeviceList, 206, SendVibrationValues, 303,
    ActivateSevenSixAxisSensor, 1000, SetNpadCommunicationMode)

result_t IHidServer::CreateAppletResource(RequestContext* ctx, u64 aruid) {
    AddService(*ctx, new IAppletResource());

    return RESULT_SUCCESS;
}

result_t IHidServer::GetSupportedNpadStyleSet(
    ::hydra::horizon::hid::NpadStyleSet* style_set) {
    // TODO: make this configurable?
    *style_set = ::hydra::horizon::hid::NpadStyleSet::Standard;

    return RESULT_SUCCESS;
}

result_t IHidServer::AcquireNpadStyleSetUpdateEventHandle(
    kernel::Process* process, u32 id, u32 _pad, u64 aruid, u64 event_ptr,
    OutHandle<HandleAttr::Copy> out_handle) {
    // TODO: params
    out_handle = process->AddHandle(npad_style_set_update_event);

    return RESULT_SUCCESS;
}

result_t
IHidServer::GetPlayerLedPattern(::hydra::horizon::hid::NpadIdType npad_id_type,
                                u64* out_pattern) {
    switch (npad_id_type) {
    case ::hydra::horizon::hid::NpadIdType::No1:
        *out_pattern = 0b0001;
        break;
    case ::hydra::horizon::hid::NpadIdType::No2:
        *out_pattern = 0b0011;
        break;
    case ::hydra::horizon::hid::NpadIdType::No3:
        *out_pattern = 0b0111;
        break;
    case ::hydra::horizon::hid::NpadIdType::No4:
        *out_pattern = 0b1111;
        break;
    case ::hydra::horizon::hid::NpadIdType::No5:
        *out_pattern = 0b1001;
        break;
    case ::hydra::horizon::hid::NpadIdType::No6:
        *out_pattern = 0b0101;
        break;
    case ::hydra::horizon::hid::NpadIdType::No7:
        *out_pattern = 0b1101;
        break;
    case ::hydra::horizon::hid::NpadIdType::No8:
        *out_pattern = 0b0110;
        break;
    case ::hydra::horizon::hid::NpadIdType::Handheld:
        *out_pattern = 0b0000;
        break;
    case ::hydra::horizon::hid::NpadIdType::Other:
        *out_pattern = 0b0000;
        break;
    }

    return RESULT_SUCCESS;
}

result_t
IHidServer::SetNpadJoyHoldType(::hydra::horizon::hid::NpadJoyHoldType type,
                               i64 aruid) {
    npad_joy_hold_type = type;
    return RESULT_SUCCESS;
}

result_t IHidServer::GetNpadJoyHoldType(
    i64 aruid, ::hydra::horizon::hid::NpadJoyHoldType* out_type) {
    *out_type = npad_joy_hold_type;
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

result_t IHidServer::CreateActiveVibrationDeviceList(RequestContext* ctx) {
    AddService(*ctx, new IActiveVibrationDeviceList());

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
