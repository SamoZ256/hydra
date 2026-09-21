#include "core/horizon/services/account/baas/manager_for_application.hpp"

namespace hydra::horizon::services::account::baas {

DEFINE_SERVICE_COMMAND_TABLE(IManagerForApplication, 0, checkAvailability, 1,
                             getAccountId)

result_t IManagerForApplication::getAccountId(u64* out_id) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_id = "net_id"_u64;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account::baas
