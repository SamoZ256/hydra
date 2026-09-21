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
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t createAppletResource(RequestContext* ctx,
                                  kernel::AppletResourceUserId aruid);
    STUB_REQUEST_COMMAND(activateDebugPad);
    STUB_REQUEST_COMMAND(activateTouchScreen);
    STUB_REQUEST_COMMAND(activateMouse);
    STUB_REQUEST_COMMAND(activateKeyboard);
    STUB_REQUEST_COMMAND(startSixAxisSensor);
    STUB_REQUEST_COMMAND(stopSixAxisSensor);
    STUB_REQUEST_COMMAND(enableSixAxisSensorFusion);
    STUB_REQUEST_COMMAND(setGyroscopeZeroDriftMode);
    STUB_REQUEST_COMMAND(activateGesture);
    result_t setSupportedNpadStyleSet(System* system,
                                      Aligned<NpadStyleSet, 8> style_set,
                                      kernel::AppletResourceUserId aruid);
    result_t getSupportedNpadStyleSet(System* system,
                                      kernel::AppletResourceUserId aruid,
                                      NpadStyleSet* out_style_set);
    result_t
    setSupportedNpadIdType(System* system, kernel::AppletResourceUserId aruid,
                           InBuffer<BufferAttr::HipcPointer> in_types_buffer);
    result_t activateNpad(System* system, kernel::AppletResourceUserId aruid);
    result_t acquireNpadStyleSetUpdateEventHandle(
        System* system, kernel::Process* process, Aligned<NpadIdType, 8> type,
        kernel::AppletResourceUserId aruid, u64 event_ptr,
        OutHandle<HandleAttr::Copy> out_handle);
    result_t disconnectNpad(System* system, Aligned<NpadIdType, 8> type,
                            kernel::AppletResourceUserId aruid);
    result_t getPlayerLedPattern(NpadIdType npad_id_type, u64* out_pattern);
    result_t activateNpadWithRevision(System* system,
                                      Aligned<NpadRevision, 8> revision,
                                      kernel::AppletResourceUserId aruid);
    // TODO: PID descriptor
    result_t setNpadJoyHoldType(System* system,
                                kernel::AppletResourceUserId aruid,
                                NpadJoyHoldType type);
    // TODO: PID descriptor
    result_t getNpadJoyHoldType(System* system,
                                kernel::AppletResourceUserId aruid,
                                Aligned<NpadJoyHoldType, 8>* out_type);
    STUB_REQUEST_COMMAND(setNpadJoyAssignmentModeSingleByDefault);
    STUB_REQUEST_COMMAND(setNpadJoyAssignmentModeDual);
    STUB_REQUEST_COMMAND(setNpadHandheldActivationMode);
    STUB_REQUEST_COMMAND(swapNpadAssignment);
    result_t getVibrationDeviceInfo(VibrationDeviceHandle handle,
                                    VibrationDeviceInfo* out_info);
    STUB_REQUEST_COMMAND(sendVibrationValue);
    result_t createActiveVibrationDeviceList(RequestContext* ctx);
    result_t isVibrationPermitted(bool* out_permitted);
    STUB_REQUEST_COMMAND(sendVibrationValues);
    STUB_REQUEST_COMMAND(activateSevenSixAxisSensor); // 5.0.0+
    STUB_REQUEST_COMMAND(setNpadCommunicationMode);
    STUB_REQUEST_COMMAND(setTouchScreenOutputRanges); // 20.0.0+
};

} // namespace hydra::horizon::services::hid
