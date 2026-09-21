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
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    kernel::Event* library_applet_launchable_event;
    kernel::Event* accumulated_suspended_tick_changed_event;

    // Commands
    result_t exit(kernel::Process* process);
    result_t lockExit(kernel::Process* process);
    result_t unlockExit(kernel::Process* process);
    result_t
    getLibraryAppletLaunchableEvent(kernel::Process* process,
                                    OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(setScreenShotPermission);
    STUB_REQUEST_COMMAND(setOperationModeChangedNotification);
    STUB_REQUEST_COMMAND(setPerformanceModeChangedNotification);
    STUB_REQUEST_COMMAND(setFocusHandlingMode);
    STUB_REQUEST_COMMAND(setRestartMessageEnabled);
    STUB_REQUEST_COMMAND(setOutOfFocusSuspendingEnabled);
    STUB_REQUEST_COMMAND(setAlbumImageOrientation);
    result_t createManagedDisplayLayer(System* system, kernel::Process* process,
                                       u64* out_layer_id);
    result_t isSystemBufferSharingEnabled();
    result_t createManagedDisplaySeparableLayer(System* system,
                                                kernel::Process* process,
                                                u64* out_display_layer_id,
                                                u64* out_recording_layer_id);
    STUB_REQUEST_COMMAND(setHandlesRequestToDisplay);
    result_t setIdleTimeDetectionExtension(IdleTimeDetectionExtension ext);
    STUB_REQUEST_COMMAND(setWirelessPriorityMode);
    result_t getAccumulatedSuspendedTickChangedEvent(
        kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am

ENABLE_ENUM_FORMATTING(hydra::horizon::services::am::IdleTimeDetectionExtension,
                       Disabled, "disabled", Extended, "extended",
                       ExtendedUnsafe, "extended unsafe")
