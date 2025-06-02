#include "core/horizon/services/pctl/parental_control_service.hpp"

namespace hydra::horizon::services::pctl {

DEFINE_SERVICE_COMMAND_TABLE(IParentalControlService, 1, Initialize, 1001,
                             CheckFreeCommunicationPermission)

result_t IParentalControlService::Initialize() {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

result_t IParentalControlService::CheckFreeCommunicationPermission() {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pctl
