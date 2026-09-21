#include "core/horizon/services/am/process_winding_controller.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IProcessWindingController, 0, getLaunchReason)

result_t IProcessWindingController::getLaunchReason(
    AppletProcessLaunchReason* out_reason) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_reason = {
        .flag = 0,
        ._unknown_x1 = 0,
        ._unknown_x3 = 0,
    };
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
