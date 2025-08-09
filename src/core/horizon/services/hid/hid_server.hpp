#pragma once

#include "core/horizon/hid.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::hid {

struct VibrationDeviceHandle {
    u32 type_value;
    u8 npad_style_index;
    ::hydra::horizon::hid::NpadIdType player_number;
    u8 device_index;
    u8 pad;
};

enum class VibrationDeviceType : u32 {
    Unknown,
    LinearResonantActuator,
    GcErm,
    Erm,
};

enum class VibrationDevicePosition : u32 {
    None,
    Left,
    Right,
};

struct VibrationDeviceInfo {
    VibrationDeviceType device_type;
    VibrationDevicePosition position;
};

class IHidServer : public IService {
  public:
    IHidServer()
        : npad_style_set_update_event{
              new kernel::Event(false, "Npad style set update event")} {}

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // TODO: one event for each style set
    kernel::Event* npad_style_set_update_event;

    ::hydra::horizon::hid::NpadJoyHoldType npad_joy_hold_type{
        ::hydra::horizon::hid::NpadJoyHoldType::Horizontal};

    // Commands
    result_t CreateAppletResource(RequestContext* ctx, u64 aruid);
    STUB_REQUEST_COMMAND(ActivateDebugPad);
    STUB_REQUEST_COMMAND(ActivateTouchScreen);
    STUB_REQUEST_COMMAND(ActivateMouse);
    STUB_REQUEST_COMMAND(ActivateKeyboard);
    STUB_REQUEST_COMMAND(StartSixAxisSensor);
    STUB_REQUEST_COMMAND(StopSixAxisSensor);
    STUB_REQUEST_COMMAND(SetGyroscopeZeroDriftMode);
    STUB_REQUEST_COMMAND(SetSupportedNpadStyleSet);
    result_t
    GetSupportedNpadStyleSet(::hydra::horizon::hid::NpadStyleSet* style_set);
    STUB_REQUEST_COMMAND(SetSupportedNpadIdType);
    STUB_REQUEST_COMMAND(ActivateNpad);
    result_t AcquireNpadStyleSetUpdateEventHandle(
        kernel::Process* process, u32 id, u32 _pad, u64 aruid, u64 event_ptr,
        OutHandle<HandleAttr::Copy> out_handle);
    result_t GetPlayerLedPattern(::hydra::horizon::hid::NpadIdType npad_id_type,
                                 u64* out_pattern);
    STUB_REQUEST_COMMAND(ActivateNpadWithRevision);
    // TODO: PID descriptor
    result_t SetNpadJoyHoldType(::hydra::horizon::hid::NpadJoyHoldType type,
                                i64 aruid);
    // TODO: PID descriptor
    result_t
    GetNpadJoyHoldType(i64 aruid,
                       ::hydra::horizon::hid::NpadJoyHoldType* out_type);
    STUB_REQUEST_COMMAND(SetNpadJoyAssignmentModeSingleByDefault);
    STUB_REQUEST_COMMAND(SetNpadJoyAssignmentModeDual);
    STUB_REQUEST_COMMAND(SetNpadHandheldActivationMode);
    STUB_REQUEST_COMMAND(SwapNpadAssignment);
    result_t GetVibrationDeviceInfo(VibrationDeviceHandle handle,
                                    VibrationDeviceInfo* info);
    STUB_REQUEST_COMMAND(SendVibrationValue);
    result_t CreateActiveVibrationDeviceList(RequestContext* ctx);
    STUB_REQUEST_COMMAND(SendVibrationValues);
    STUB_REQUEST_COMMAND(ActivateSevenSixAxisSensor); // 5.0.0+
    STUB_REQUEST_COMMAND(SetNpadCommunicationMode);
};

} // namespace hydra::horizon::services::hid
