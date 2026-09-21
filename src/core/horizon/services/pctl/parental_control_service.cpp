#include "core/horizon/services/pctl/parental_control_service.hpp"

namespace hydra::horizon::services::pctl {

DEFINE_SERVICE_COMMAND_TABLE(IParentalControlService, 1, initialize, 1001,
                             checkFreeCommunicationPermission)

result_t IParentalControlService::initialize() {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

result_t IParentalControlService::checkFreeCommunicationPermission() {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pctl
