#include "core/horizon/services/lm/log_service.hpp"

#include "core/horizon/services/lm/logger.hpp"

namespace Hydra::Horizon::Services::Lm {

DEFINE_SERVICE_COMMAND_TABLE(ILogService, 0, OpenLogger)

result_t ILogService::OpenLogger(add_service_fn_t add_service, u64 process_id) {
    // TODO: what is process ID for?
    LOG_DEBUG(HorizonServices, "Process ID: {}", process_id);

    add_service(new ILogger());
    return RESULT_SUCCESS;
}

} // namespace Hydra::Horizon::Services::Lm
