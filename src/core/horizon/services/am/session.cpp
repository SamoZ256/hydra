#include "core/horizon/services/am/session.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ISession, 0, SetPerformanceConfiguration)

result_t ISession::SetPerformanceConfiguration(PerformanceMode mode,
                                               u32 config) {
    LOG_DEBUG(Services, "Mode: {}, configuration: 0x{:08x}", mode, config);

    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
