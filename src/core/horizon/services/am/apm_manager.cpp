#include "core/horizon/services/am/apm_manager.hpp"

#include "core/horizon/services/am/session.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IApmManager, 0, OpenSession)

result_t IApmManager::OpenSession(add_service_fn_t add_service) {
    add_service(new ISession());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
