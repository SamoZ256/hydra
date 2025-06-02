#include "core/horizon/services/account/manager_for_application.hpp"

namespace hydra::horizon::services::account {

DEFINE_SERVICE_COMMAND_TABLE(IManagerForApplication, 0, CheckAvailability, 1,
                             GetAccountId)

result_t IManagerForApplication::GetAccountId(u64* out_id) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_id = str_to_u64("net ID");
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account
