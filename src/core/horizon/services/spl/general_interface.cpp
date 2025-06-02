#include "core/horizon/services/spl/general_interface.hpp"

namespace hydra::horizon::services::spl {

DEFINE_SERVICE_COMMAND_TABLE(IGeneralInterface, 0, GetConfig)

result_t IGeneralInterface::GetConfig(ConfigItem item, u64* out_config) {
    LOG_DEBUG(Services, "Config item: {}", item);

    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_config = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::spl
