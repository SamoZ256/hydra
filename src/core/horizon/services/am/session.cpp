#include "core/horizon/services/am/session.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ISession, 0, setPerformanceConfiguration, 1,
                             getPerformanceConfiguration)

result_t ISession::setPerformanceConfiguration(PerformanceMode mode,
                                               u32 config) {
    LOG_DEBUG(Services, "Mode: {}, configuration: 0x{:08x}", mode, config);

    ASSERT_DEBUG(mode != PerformanceMode::Invalid, Services,
                 "Invalid performance mode");
    performance_configs[static_cast<u32>(mode)] = config;
    return RESULT_SUCCESS;
}

result_t ISession::getPerformanceConfiguration(PerformanceMode mode,
                                               u32* out_config) {
    LOG_DEBUG(Services, "Mode: {}", mode);

    ASSERT_DEBUG(mode != PerformanceMode::Invalid, Services,
                 "Invalid performance mode");
    *out_config = performance_configs[static_cast<u32>(mode)];
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
