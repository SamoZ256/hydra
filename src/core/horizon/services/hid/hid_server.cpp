#include "core/horizon/services/hid/hid_server.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/hid/active_vibration_device_list.hpp"
#include "core/horizon/services/hid/applet_resource.hpp"
#include "core/system.hpp"

#define APPLET_RESOURCE(aruid)                                                 \
    system->getOs().getHidResourceManager().getResource(aruid)

namespace hydra::horizon::services::hid {

DEFINE_SERVICE_COMMAND_TABLE(
    IHidServer, 0, createAppletResource, 1, activateDebugPad, 11,
    activateTouchScreen, 21, activateMouse, 31, activateKeyboard, 66,
    startSixAxisSensor, 67, stopSixAxisSensor, 69, enableSixAxisSensorFusion,
    79, setGyroscopeZeroDriftMode, 91, activateGesture, 100,
    setSupportedNpadStyleSet, 101, getSupportedNpadStyleSet, 102,
    setSupportedNpadIdType, 103, activateNpad, 106,
    acquireNpadStyleSetUpdateEventHandle, 107, disconnectNpad, 108,
    getPlayerLedPattern, 109, activateNpadWithRevision, 120, setNpadJoyHoldType,
    121, getNpadJoyHoldType, 122, setNpadJoyAssignmentModeSingleByDefault, 124,
    setNpadJoyAssignmentModeDual, 128, setNpadHandheldActivationMode, 130,
    swapNpadAssignment, 200, getVibrationDeviceInfo, 201, sendVibrationValue,
    203, createActiveVibrationDeviceList, 205, isVibrationPermitted, 206,
    sendVibrationValues, 303, activateSevenSixAxisSensor, 1000,
    setNpadCommunicationMode, 1004, setTouchScreenOutputRanges)

result_t IHidServer::createAppletResource(RequestContext* ctx,
                                          kernel::AppletResourceUserId aruid) {
    addService(*ctx, new IAppletResource(ctx->system, aruid));
    return RESULT_SUCCESS;
}

result_t
IHidServer::setSupportedNpadStyleSet(System* system,
                                     Aligned<NpadStyleSet, 8> style_set,
                                     kernel::AppletResourceUserId aruid) {
    APPLET_RESOURCE(aruid).setSupportedStyleSet(style_set);
    return RESULT_SUCCESS;
}

result_t
IHidServer::getSupportedNpadStyleSet(System* system,
                                     kernel::AppletResourceUserId aruid,
                                     NpadStyleSet* out_style_set) {
    *out_style_set = APPLET_RESOURCE(aruid).getSupportedStyleSet();
    return RESULT_SUCCESS;
}

result_t IHidServer::setSupportedNpadIdType(
    System* system, kernel::AppletResourceUserId aruid,
    InBuffer<BufferAttr::HipcPointer> in_types_buffer) {
    while (in_types_buffer.stream->getSeek() <
           in_types_buffer.stream->getSize()) {
        const auto index =
            internal::toNpadIndex(in_types_buffer.stream->read<NpadIdType>());
        APPLET_RESOURCE(aruid).setNpadSupported(index, true);
    }
    return RESULT_SUCCESS;
}

result_t IHidServer::activateNpad(System* system,
                                  kernel::AppletResourceUserId aruid) {
    APPLET_RESOURCE(aruid).activateNpads(NpadRevision::Revision0);
    return RESULT_SUCCESS;
}

result_t IHidServer::acquireNpadStyleSetUpdateEventHandle(
    System* system, kernel::Process* process, Aligned<NpadIdType, 8> type,
    kernel::AppletResourceUserId aruid, u64 event_ptr,
    OutHandle<HandleAttr::Copy> out_handle) {
    (void)event_ptr;
    LOG_DEBUG(Services, "event ptr: {:#x}", event_ptr);

    auto event = APPLET_RESOURCE(aruid).getNpadStyleSetUpdateEvent(
        internal::toNpadIndex(type));

    // TODO: params
    out_handle = process->addHandle(event);

    // HACK: games expect this to be signalled
    event->signal();

    return RESULT_SUCCESS;
}

result_t IHidServer::disconnectNpad(System* system, Aligned<NpadIdType, 8> type,
                                    kernel::AppletResourceUserId aruid) {
    APPLET_RESOURCE(aruid).disconnectNpad(internal::toNpadIndex(type));
    return RESULT_SUCCESS;
}

result_t IHidServer::getPlayerLedPattern(NpadIdType npad_id_type,
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
IHidServer::activateNpadWithRevision(System* system,
                                     Aligned<NpadRevision, 8> revision,
                                     kernel::AppletResourceUserId aruid) {
    LOG_DEBUG(Services, "Revision: {}", revision);
    APPLET_RESOURCE(aruid).activateNpads(revision);
    return RESULT_SUCCESS;
}

result_t IHidServer::setNpadJoyHoldType(System* system,
                                        kernel::AppletResourceUserId aruid,
                                        NpadJoyHoldType type) {
    APPLET_RESOURCE(aruid).setJoyHoldType(type);
    return RESULT_SUCCESS;
}

result_t IHidServer::getNpadJoyHoldType(System* system,
                                        kernel::AppletResourceUserId aruid,
                                        Aligned<NpadJoyHoldType, 8>* out_type) {
    out_type->zeroOutPadding();
    *out_type = APPLET_RESOURCE(aruid).getJoyHoldType();
    return RESULT_SUCCESS;
}

result_t IHidServer::getVibrationDeviceInfo(VibrationDeviceHandle handle,
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

result_t IHidServer::createActiveVibrationDeviceList(RequestContext* ctx) {
    addService(*ctx, new IActiveVibrationDeviceList());

    return RESULT_SUCCESS;
}

result_t IHidServer::isVibrationPermitted(bool* out_permitted) {
    // TODO: make this configurable
    *out_permitted = true;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::hid
