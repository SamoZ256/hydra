#include "core/horizon/services/account/account_service_for_administrator.hpp"

namespace hydra::horizon::services::account {

DEFINE_SERVICE_COMMAND_TABLE(IAccountServiceForAdministrator, 0, getUserCount,
                             1, getUserExistence, 2, listAllUsers, 3,
                             listOpenUsers, 4, getLastOpenedUser, 5, getProfile,
                             50, isUserRegistrationRequestPermitted)

} // namespace hydra::horizon::services::account
