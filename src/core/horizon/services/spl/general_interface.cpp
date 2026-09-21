#include "core/horizon/services/spl/general_interface.hpp"

namespace hydra::horizon::services::spl {

DEFINE_SERVICE_COMMAND_TABLE(IGeneralInterface, 0, getConfig)

result_t IGeneralInterface::getConfig(ConfigItem item, u64* out_config) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "config item: {}", item);

    // HACK
    *out_config = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::spl
