#include "core/horizon/services/hid/hid_server.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/hid/active_vibration_device_list.hpp"
#include "core/horizon/services/hid/applet_resource.hpp"
#include "core/system.hpp"

#define APPLET_RESOURCE(aruid)                                                 \
    system->GetOS().GetHidResourceManager().GetResource(aruid)

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(
    IHidServer, 0, CreateAppletResource, 1, ActivateDebugPad, 11,
    ActivateTouchScreen, 21, ActivateMouse, 31, ActivateKeyboard, 66,
    StartSixAxisSensor, 67, StopSixAxisSensor, 69, EnableSixAxisSensorFusion,
    79, SetGyroscopeZeroDriftMode, 91, ActivateGesture, 100,
    SetSupportedNpadStyleSet, 101, GetSupportedNpadStyleSet, 102,
    SetSupportedNpadIdType, 103, ActivateNpad, 106,
    AcquireNpadStyleSetUpdateEventHandle, 107, DisconnectNpad, 108,
    GetPlayerLedPattern, 109, ActivateNpadWithRevision, 120, SetNpadJoyHoldType,
    121, GetNpadJoyHoldType, 122, SetNpadJoyAssignmentModeSingleByDefault, 124,
    SetNpadJoyAssignmentModeDual, 128, SetNpadHandheldActivationMode, 130,
    SwapNpadAssignment, 200, GetVibrationDeviceInfo, 201, SendVibrationValue,
    203, CreateActiveVibrationDeviceList, 205, IsVibrationPermitted, 206,
    SendVibrationValues, 303, ActivateSevenSixAxisSensor, 1000,
    SetNpadCommunicationMode, 1004, SetTouchScreenOutputRanges)

result_t IHidServer::CreateAppletResource(RequestContext* ctx,
                                          kernel::AppletResourceUserId aruid) {
    AddService(*ctx, new IAppletResource(ctx->system, aruid));
    return RESULT_SUCCESS;
}

result_t
IHidServer::SetSupportedNpadStyleSet(System* system,
                                     aligned<NpadStyleSet, 8> style_set,
                                     kernel::AppletResourceUserId aruid) {
    APPLET_RESOURCE(aruid).SetSupportedStyleSet(style_set);
    return RESULT_SUCCESS;
}

result_t
IHidServer::GetSupportedNpadStyleSet(System* system,
                                     kernel::AppletResourceUserId aruid,
                                     NpadStyleSet* out_style_set) {
    *out_style_set = APPLET_RESOURCE(aruid).GetSupportedStyleSet();
    return RESULT_SUCCESS;
}

result_t IHidServer::SetSupportedNpadIdType(
    System* system, kernel::AppletResourceUserId aruid,
    InBuffer<BufferAttr::HipcPointer> in_types_buffer) {
    while (in_types_buffer.stream->GetSeek() <
           in_types_buffer.stream->GetSize()) {
        const auto index =
            internal::ToNpadIndex(in_types_buffer.stream->Read<NpadIdType>());
        APPLET_RESOURCE(aruid).SetNpadSupported(index, true);
    }
    return RESULT_SUCCESS;
}

result_t IHidServer::ActivateNpad(System* system,
                                  kernel::AppletResourceUserId aruid) {
    APPLET_RESOURCE(aruid).ActivateNpads(NpadRevision::Revision0);
    return RESULT_SUCCESS;
}

result_t IHidServer::AcquireNpadStyleSetUpdateEventHandle(
    System* system, kernel::Process* process, aligned<NpadIdType, 8> type,
    kernel::AppletResourceUserId aruid, u64 event_ptr,
    OutHandle<HandleAttr::Copy> out_handle) {
    (void)event_ptr;
    LOG_DEBUG(Services, "event ptr: {:#x}", event_ptr);

    auto event = APPLET_RESOURCE(aruid).GetNpadStyleSetUpdateEvent(
        internal::ToNpadIndex(type));

    // TODO: params
    out_handle = process->AddHandle(event);

    // HACK: games expect this to be signalled
    event->Signal();

    return RESULT_SUCCESS;
}

result_t IHidServer::DisconnectNpad(System* system, aligned<NpadIdType, 8> type,
                                    kernel::AppletResourceUserId aruid) {
    APPLET_RESOURCE(aruid).DisconnectNpad(internal::ToNpadIndex(type));
    return RESULT_SUCCESS;
}

result_t IHidServer::GetPlayerLedPattern(NpadIdType npad_id_type,
                                         u64* out_pattern) {
    switch (npad_id_type) {
    case NpadIdType::No1:
        *out_pattern = 0b0001;
        break;
    case NpadIdType::No2:
        *out_pattern = 0b0011;
        break;
    case NpadIdType::No3:
        *out_pattern = 0b0111;
        break;
    case NpadIdType::No4:
        *out_pattern = 0b1111;
        break;
    case NpadIdType::No5:
        *out_pattern = 0b1001;
        break;
    case NpadIdType::No6:
        *out_pattern = 0b0101;
        break;
    case NpadIdType::No7:
        *out_pattern = 0b1101;
        break;
    case NpadIdType::No8:
        *out_pattern = 0b0110;
        break;
    case NpadIdType::Handheld:
        *out_pattern = 0b0000;
        break;
    case NpadIdType::Other:
        *out_pattern = 0b0000;
        break;
    default:
        LOG_WARN(Services, "Invalid npad ID type {}", npad_id_type);
        *out_pattern = 0b0000;
        break;
    }

    return RESULT_SUCCESS;
}

result_t
IHidServer::ActivateNpadWithRevision(System* system,
                                     aligned<NpadRevision, 8> revision,
                                     kernel::AppletResourceUserId aruid) {
    LOG_DEBUG(Services, "Revision: {}", revision);
    APPLET_RESOURCE(aruid).ActivateNpads(revision);
    return RESULT_SUCCESS;
}

result_t IHidServer::SetNpadJoyHoldType(System* system,
                                        kernel::AppletResourceUserId aruid,
                                        NpadJoyHoldType type) {
    APPLET_RESOURCE(aruid).SetJoyHoldType(type);
    return RESULT_SUCCESS;
}

result_t IHidServer::GetNpadJoyHoldType(System* system,
                                        kernel::AppletResourceUserId aruid,
                                        aligned<NpadJoyHoldType, 8>* out_type) {
    out_type->ZeroOutPadding();
    *out_type = APPLET_RESOURCE(aruid).GetJoyHoldType();
    return RESULT_SUCCESS;
}

result_t IHidServer::GetVibrationDeviceInfo(VibrationDeviceHandle handle,
                                            VibrationDeviceInfo* out_info) {
    (void)handle;

    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_info = {
        .device_type = VibrationDeviceType::LinearResonantActuator,
        .position = VibrationDevicePosition::Left,
    };

    return RESULT_SUCCESS;
}

result_t IHidServer::CreateActiveVibrationDeviceList(RequestContext* ctx) {
    AddService(*ctx, new IActiveVibrationDeviceList());

    return RESULT_SUCCESS;
}

result_t IHidServer::IsVibrationPermitted(bool* out_permitted) {
    // TODO: make this configurable
    *out_permitted = true;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
