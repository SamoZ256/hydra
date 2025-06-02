#include "core/horizon/services/am/window_controller.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IWindowController, 1, GetAppletResourceUserId, 10,
                             AcquireForegroundRights)

result_t IWindowController::GetAppletResourceUserId(u64* out_aruid) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_aruid = 1;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
