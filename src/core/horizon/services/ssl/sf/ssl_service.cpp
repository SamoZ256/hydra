#include "core/horizon/services/ssl/sf/ssl_service.hpp"

namespace hydra::horizon::services::ssl::sf {

DEFINE_SERVICE_COMMAND_TABLE(ISslService, 5, SetInterfaceVersion)

result_t ISslService::SetInterfaceVersion(SystemVersion version) {
    LOG_FUNC_STUBBED(Services);

    LOG_DEBUG(Services, "Version: {}", version);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ssl::sf
