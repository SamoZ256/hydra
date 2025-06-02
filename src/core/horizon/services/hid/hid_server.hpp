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

class IHidServer : public ServiceBase {
  public:
    IHidServer();

    usize GetPointerBufferSize() override { return 0x1000; }

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // TODO: one event for each style set
    kernel::HandleWithId<kernel::Event> npad_style_set_update_event;

    // Commands
    result_t CreateAppletResource(kernel::add_service_fn_t add_service,
                                  u64 aruid);
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
        u32 id, u32 _pad, u64 aruid, u64 event_ptr,
        OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(ActivateNpadWithRevision);
    STUB_REQUEST_COMMAND(SetNpadJoyHoldType);
    STUB_REQUEST_COMMAND(SetNpadJoyAssignmentModeSingleByDefault);
    STUB_REQUEST_COMMAND(SetNpadJoyAssignmentModeDual);
    STUB_REQUEST_COMMAND(SetNpadHandheldActivationMode);
    result_t GetVibrationDeviceInfo(VibrationDeviceHandle handle,
                                    VibrationDeviceInfo* info);
    STUB_REQUEST_COMMAND(SendVibrationValue);
    result_t
    CreateActiveVibrationDeviceList(kernel::add_service_fn_t add_service);
    STUB_REQUEST_COMMAND(SendVibrationValues);
};

} // namespace hydra::horizon::services::hid
