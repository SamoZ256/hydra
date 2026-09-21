#include "core/horizon/services/lm/log_service.hpp"

#include "core/horizon/services/lm/logger.hpp"

namespace hydra::horizon::services::lm {

DEFINE_SERVICE_COMMAND_TABLE(ILogService, 0, openLogger)

result_t ILogService::openLogger(RequestContext* ctx, u64 process_id) {
    // TODO: what is process ID for?
    (void)process_id;
    LOG_DEBUG(Services, "Process ID: {}", process_id);

    addService(*ctx, new ILogger());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::lm
