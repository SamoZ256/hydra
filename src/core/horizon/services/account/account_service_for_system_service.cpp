#include "core/horizon/services/account/account_service_for_system_service.hpp"

#include "core/horizon/services/account/baas/manager_for_system_service.hpp"
#include "core/horizon/services/account/notifier.hpp"

namespace hydra::horizon::services::account {

DEFINE_SERVICE_COMMAND_TABLE(IAccountServiceForSystemService, 0, getUserCount,
                             1, getUserExistence, 2, listAllUsers, 3,
                             listOpenUsers, 4, getLastOpenedUser, 5, getProfile,
                             100, getUserRegistrationNotifier, 101,
                             getUserStateChangeNotifier, 102,
                             getBaasAccountManagerForSystemService)

result_t IAccountServiceForSystemService::getUserRegistrationNotifier(
    RequestContext* ctx) {
    addService(*ctx, new INotifier());
    return RESULT_SUCCESS;
}

result_t IAccountServiceForSystemService::getUserStateChangeNotifier(
    RequestContext* ctx) {
    addService(*ctx, new INotifier());
    return RESULT_SUCCESS;
}

result_t IAccountServiceForSystemService::getBaasAccountManagerForSystemService(
    RequestContext* ctx) {
    addService(*ctx, new baas::IManagerForSystemService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account
