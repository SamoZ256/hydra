#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

enum class IdleTimeDetectionExtension : u32 {
    Disabled = 0,
    Extended = 1,
    ExtendedUnsafe = 2,
};

class ISelfController : public IService {
  public:
    ISelfController();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::Event* library_applet_launchable_event;
    kernel::Event* accumulated_suspended_tick_changed_event;

    // Commands
    STUB_REQUEST_COMMAND(Exit);
    result_t LockExit(kernel::Process* process);
    result_t UnlockExit(kernel::Process* process);
    result_t
    GetLibraryAppletLaunchableEvent(kernel::Process* process,
                                    OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(SetScreenShotPermission);
    STUB_REQUEST_COMMAND(SetOperationModeChangedNotification);
    STUB_REQUEST_COMMAND(SetPerformanceModeChangedNotification);
    STUB_REQUEST_COMMAND(SetFocusHandlingMode);
    STUB_REQUEST_COMMAND(SetRestartMessageEnabled);
    STUB_REQUEST_COMMAND(SetOutOfFocusSuspendingEnabled);
    STUB_REQUEST_COMMAND(SetAlbumImageOrientation);
    result_t CreateManagedDisplayLayer(kernel::Process* process,
                                       u64* out_layer_id);
    result_t CreateManagedDisplaySeparableLayer(kernel::Process* process,
                                                u64* out_display_layer_id,
                                                u64* out_recording_layer_id);
    STUB_REQUEST_COMMAND(SetHandlesRequestToDisplay);
    result_t SetIdleTimeDetectionExtension(IdleTimeDetectionExtension ext);
    STUB_REQUEST_COMMAND(SetWirelessPriorityMode);
    result_t GetAccumulatedSuspendedTickChangedEvent(
        kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am

ENABLE_ENUM_FORMATTING(hydra::horizon::services::am::IdleTimeDetectionExtension,
                       Disabled, "disabled", Extended, "extended",
                       ExtendedUnsafe, "extended unsafe")
