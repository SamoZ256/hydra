#include "core/horizon/services/am/self_controller.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(
    ISelfController, 0, Exit, 1, LockExit, 2, UnlockExit, 9,
    GetLibraryAppletLaunchableEvent, 10, SetScreenShotPermission, 11,
    SetOperationModeChangedNotification, 12,
    SetPerformanceModeChangedNotification, 13, SetFocusHandlingMode, 14,
    SetRestartMessageEnabled, 16, SetOutOfFocusSuspendingEnabled, 19,
    SetAlbumImageOrientation, 40, CreateManagedDisplayLayer, 44,
    CreateManagedDisplaySeparableLayer, 50, SetHandlesRequestToDisplay, 62,
    SetIdleTimeDetectionExtension, 80, SetWirelessPriorityMode, 91,
    GetAccumulatedSuspendedTickChangedEvent)

ISelfController::ISelfController()
    : library_applet_launchable_event{new kernel::Event(
          true, "Library applet launchable event")},
      accumulated_suspended_tick_changed_event{new kernel::Event(
          false, "Accumulated suspended tick changed event")} {}

result_t ISelfController::LockExit(kernel::Process* process) {
    process->GetAppletState().LockExit();
    return RESULT_SUCCESS;
}

result_t ISelfController::UnlockExit(kernel::Process* process) {
    process->GetAppletState().UnlockExit();
    return RESULT_SUCCESS;
}

result_t ISelfController::GetLibraryAppletLaunchableEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(library_applet_launchable_event);
    return RESULT_SUCCESS;
}

result_t ISelfController::CreateManagedDisplayLayer(kernel::Process* process,
                                                    u64* out_layer_id) {
    u32 binder_id = OS_INSTANCE.GetDisplayDriver().CreateBinder();
    *out_layer_id =
        OS_INSTANCE.GetDisplayDriver().CreateLayer(process, binder_id);
    return RESULT_SUCCESS;
}

result_t ISelfController::CreateManagedDisplaySeparableLayer(
    kernel::Process* process, u64* out_display_layer_id,
    u64* out_recording_layer_id) {
    u32 binder_id = OS_INSTANCE.GetDisplayDriver().CreateBinder();
    *out_display_layer_id =
        OS_INSTANCE.GetDisplayDriver().CreateLayer(process, binder_id);
    // TODO: what is a recording layer?
    *out_recording_layer_id =
        OS_INSTANCE.GetDisplayDriver().CreateLayer(process, binder_id);
    return RESULT_SUCCESS;
}

result_t
ISelfController::SetIdleTimeDetectionExtension(IdleTimeDetectionExtension ext) {
    LOG_FUNC_STUBBED(Services);

    LOG_DEBUG(Services, "Extension: {}", ext);

    return RESULT_SUCCESS;
}

result_t ISelfController::GetAccumulatedSuspendedTickChangedEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(accumulated_suspended_tick_changed_event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
