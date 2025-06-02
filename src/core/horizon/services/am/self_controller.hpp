#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class ISelfController : public ServiceBase {
  public:
    ISelfController();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::HandleWithId<kernel::Event> library_applet_launchable_event;
    kernel::HandleWithId<kernel::Event>
        accumulated_suspended_tick_changed_event;

    // Commands
    STUB_REQUEST_COMMAND(Exit);
    result_t LockExit();
    result_t UnlockExit();
    result_t
    GetLibraryAppletLaunchableEvent(OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(SetScreenShotPermission);
    STUB_REQUEST_COMMAND(SetOperationModeChangedNotification);
    STUB_REQUEST_COMMAND(SetPerformanceModeChangedNotification);
    STUB_REQUEST_COMMAND(SetFocusHandlingMode);
    STUB_REQUEST_COMMAND(SetRestartMessageEnabled);
    STUB_REQUEST_COMMAND(SetOutOfFocusSuspendingEnabled);
    result_t CreateManagedDisplayLayer(u64* out_layer_id);
    result_t CreateManagedDisplaySeparableLayer(u64* out_display_layer_id,
                                                u64* out_recording_layer_id);
    result_t GetAccumulatedSuspendedTickChangedEvent(
        OutHandle<HandleAttr::Copy> out_handle);
};

} // namespace hydra::horizon::services::am
