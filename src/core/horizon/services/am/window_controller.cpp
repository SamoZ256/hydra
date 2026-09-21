#include "core/horizon/services/am/window_controller.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IWindowController, 1, getAppletResourceUserId, 10,
                             acquireForegroundRights)

result_t IWindowController::getAppletResourceUserId(kernel::Process* process,
                                                    u64* out_aruid) {
    *out_aruid = process->getAppletState().getAppletResourceUserId();
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
