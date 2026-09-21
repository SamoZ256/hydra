#include "core/horizon/services/account/account_service_for_application.hpp"

#include "core/horizon/services/account/baas/manager_for_application.hpp"

namespace hydra::horizon::services::account {

DEFINE_SERVICE_COMMAND_TABLE(
    IAccountServiceForApplication, 0, getUserCount, 1, getUserExistence, 2,
    listAllUsers, 3, listOpenUsers, 4, getLastOpenedUser, 5, getProfile, 50,
    isUserRegistrationRequestPermitted, 100, initializeApplicationInfoV0, 101,
    getBaasAccountManagerForApplication, 110, storeSaveDataThumbnail, 140,
    initializeApplicationInfo, 150, isUserAccountSwitchLocked, 160,
    initializeApplicationInfoV2)

result_t IAccountServiceForApplication::getBaasAccountManagerForApplication(
    RequestContext* ctx, uuid_t user_id) {
    addService(*ctx, new baas::IManagerForApplication(user_id));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::account
