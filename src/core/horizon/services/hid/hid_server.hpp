#pragma once

#include "core/horizon/kernel/applet_resource.hpp"
#include "core/horizon/services/const.hpp"
#include "core/horizon/services/hid/const.hpp"

namespace hydra::horizon::services::hid {

struct VibrationDeviceHandle {
    u32 type_value;
    u8 npad_style_index;
    NpadIdType player_number;
    u8 device_index;
    u8 pad;
};

struct VibrationDeviceInfo {
    VibrationDeviceType device_type;
    VibrationDevicePosition position;
};

class IHidServer : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t CreateAppletResource(RequestContext* ctx,
                                  kernel::AppletResourceUserId aruid);
    STUB_REQUEST_COMMAND(ActivateDebugPad);
    STUB_REQUEST_COMMAND(ActivateTouchScreen);
    STUB_REQUEST_COMMAND(ActivateMouse);
    STUB_REQUEST_COMMAND(ActivateKeyboard);
    STUB_REQUEST_COMMAND(StartSixAxisSensor);
    STUB_REQUEST_COMMAND(StopSixAxisSensor);
    STUB_REQUEST_COMMAND(EnableSixAxisSensorFusion);
    STUB_REQUEST_COMMAND(SetGyroscopeZeroDriftMode);
    STUB_REQUEST_COMMAND(ActivateGesture);
    result_t SetSupportedNpadStyleSet(System* system,
                                      aligned<NpadStyleSet, 8> style_set,
                                      kernel::AppletResourceUserId aruid);
    result_t GetSupportedNpadStyleSet(System* system,
                                      kernel::AppletResourceUserId aruid,
                                      NpadStyleSet* out_style_set);
    result_t
    SetSupportedNpadIdType(System* system, kernel::AppletResourceUserId aruid,
                           InBuffer<BufferAttr::HipcPointer> in_types_buffer);
    result_t ActivateNpad(System* system, kernel::AppletResourceUserId aruid);
    result_t AcquireNpadStyleSetUpdateEventHandle(
        System* system, kernel::Process* process, aligned<NpadIdType, 8> type,
        kernel::AppletResourceUserId aruid, u64 event_ptr,
        OutHandle<HandleAttr::Copy> out_handle);
    result_t DisconnectNpad(System* system, aligned<NpadIdType, 8> type,
                            kernel::AppletResourceUserId aruid);
    result_t GetPlayerLedPattern(NpadIdType npad_id_type, u64* out_pattern);
    result_t ActivateNpadWithRevision(System* system,
                                      aligned<NpadRevision, 8> revision,
                                      kernel::AppletResourceUserId aruid);
    // TODO: PID descriptor
    result_t SetNpadJoyHoldType(System* system,
                                kernel::AppletResourceUserId aruid,
                                NpadJoyHoldType type);
    // TODO: PID descriptor
    result_t GetNpadJoyHoldType(System* system,
                                kernel::AppletResourceUserId aruid,
                                aligned<NpadJoyHoldType, 8>* out_type);
    STUB_REQUEST_COMMAND(SetNpadJoyAssignmentModeSingleByDefault);
    STUB_REQUEST_COMMAND(SetNpadJoyAssignmentModeDual);
    STUB_REQUEST_COMMAND(SetNpadHandheldActivationMode);
    STUB_REQUEST_COMMAND(SwapNpadAssignment);
    result_t GetVibrationDeviceInfo(VibrationDeviceHandle handle,
                                    VibrationDeviceInfo* out_info);
    STUB_REQUEST_COMMAND(SendVibrationValue);
    result_t CreateActiveVibrationDeviceList(RequestContext* ctx);
    result_t IsVibrationPermitted(bool* out_permitted);
    STUB_REQUEST_COMMAND(SendVibrationValues);
    STUB_REQUEST_COMMAND(ActivateSevenSixAxisSensor); // 5.0.0+
    STUB_REQUEST_COMMAND(SetNpadCommunicationMode);
    STUB_REQUEST_COMMAND(SetTouchScreenOutputRanges); // 20.0.0+
};

} // namespace hydra::horizon::services::hid
