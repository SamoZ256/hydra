#include "horizon/services/am/self_controller.hpp"

#include "horizon/os.hpp"
#include "hw/bus.hpp"
#include "hw/display/display.hpp"

namespace Hydra::Horizon::Services::Am {

DEFINE_SERVICE_COMMAND_TABLE(ISelfController, 1, LockExit, 2, UnlockExit, 11,
                             SetOperationModeChangedNotification, 12,
                             SetPerformanceModeChangedNotification, 13,
                             SetFocusHandlingMode, 16,
                             SetOutOfFocusSuspendingEnabled, 40,
                             CreateManagedDisplayLayer)

void ISelfController::LockExit(REQUEST_COMMAND_PARAMS) {
    StateManager::GetInstance().LockExit();
}

void ISelfController::UnlockExit(REQUEST_COMMAND_PARAMS) {
    StateManager::GetInstance().UnlockExit();
}

void ISelfController::CreateManagedDisplayLayer(REQUEST_COMMAND_PARAMS) {
    u32 binder_id = OS::GetInstance().GetDisplayBinderManager().AddBinder();

    // TODO: what display ID should be used?
    u64 layer_id =
        Kernel::GetInstance().GetBus().GetDisplay(0)->CreateLayer(binder_id);

    writers.writer.Write(layer_id);
}

} // namespace Hydra::Horizon::Services::Am
