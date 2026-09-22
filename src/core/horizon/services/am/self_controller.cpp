#include "core/horizon/services/am/self_controller.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(
    ISelfController, 0, exit, 1, lockExit, 2, unlockExit, 9,
    getLibraryAppletLaunchableEvent, 10, setScreenShotPermission, 11,
    setOperationModeChangedNotification, 12,
    setPerformanceModeChangedNotification, 13, setFocusHandlingMode, 14,
    setRestartMessageEnabled, 16, setOutOfFocusSuspendingEnabled, 19,
    setAlbumImageOrientation, 40, createManagedDisplayLayer, 41,
    isSystemBufferSharingEnabled, 44, createManagedDisplaySeparableLayer, 50,
    setHandlesRequestToDisplay, 62, setIdleTimeDetectionExtension, 80,
    setWirelessPriorityMode, 91, getAccumulatedSuspendedTickChangedEvent)

ISelfController::ISelfController()
    : library_applet_launchable_event{
          new kernel::Event(true, "Library applet launchable event")},
      accumulated_suspended_tick_changed_event{new kernel::Event(
          false, "Accumulated suspended tick changed event")} {}

result_t ISelfController::exit(kernel::Process* process) {
    // TODO: correct?
    process->stop();
    return RESULT_SUCCESS;
}

result_t ISelfController::lockExit(kernel::Process* process) {
    process->getAppletState().lockExit();
    return RESULT_SUCCESS;
}

result_t ISelfController::unlockExit(kernel::Process* process) {
    process->getAppletState().unlockExit();
    return RESULT_SUCCESS;
}

result_t ISelfController::getLibraryAppletLaunchableEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(library_applet_launchable_event);
    return RESULT_SUCCESS;
}

result_t ISelfController::createManagedDisplayLayer(System* system,
                                                    kernel::Process* process,
                                                    u64* out_layer_id) {
    const auto binder_handle =
        system->getOs().getDisplayDriver().createBinder();
    *out_layer_id = system->getOs()
                        .getDisplayDriver()
                        .createLayer(process, binder_handle)
                        .getRaw();
    return RESULT_SUCCESS;
}

result_t ISelfController::isSystemBufferSharingEnabled() {
    // TODO: implement
    LOG_FUNC_STUBBED(Services);
    return MAKE_RESULT(Am, 998); // Not implemented
}

result_t ISelfController::createManagedDisplaySeparableLayer(
    System* system, kernel::Process* process, u64* out_display_layer_id,
    u64* out_recording_layer_id) {
    const auto binder_handle =
        system->getOs().getDisplayDriver().createBinder();
    *out_display_layer_id = system->getOs()
                                .getDisplayDriver()
                                .createLayer(process, binder_handle)
                                .getRaw();
    // TODO: what is a recording layer?
    *out_recording_layer_id = system->getOs()
                                  .getDisplayDriver()
                                  .createLayer(process, binder_handle)
                                  .getRaw();
    return RESULT_SUCCESS;
}

result_t
ISelfController::setIdleTimeDetectionExtension(IdleTimeDetectionExtension ext) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "extension: {}", ext);
    return RESULT_SUCCESS;
}

result_t ISelfController::getAccumulatedSuspendedTickChangedEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(accumulated_suspended_tick_changed_event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
