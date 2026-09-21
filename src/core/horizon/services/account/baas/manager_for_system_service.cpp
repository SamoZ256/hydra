#include "core/horizon/services/account/baas/manager_for_system_service.hpp"

namespace hydra::horizon::services::account::baas {

DEFINE_SERVICE_COMMAND_TABLE(IManagerForSystemService, 0, checkAvailability, 1,
                             getAccountId)

result_t IManagerForSystemService::getAccountId(uuid_t* out_id) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_id = 0x23232323;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account::baas
