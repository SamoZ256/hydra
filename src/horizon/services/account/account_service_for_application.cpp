#include "horizon/services/account/account_service_for_application.hpp"
#include "horizon/services/account/profile.hpp"

namespace Hydra::Horizon::Services::Account {

DEFINE_SERVICE_COMMAND_TABLE(IAccountServiceForApplication, 5, GetProfile, 100,
                             InitializeApplicationInfoV0)

void IAccountServiceForApplication::GetProfile(REQUEST_COMMAND_PARAMS) {
    const u128 account_uid = readers.reader.Read<u128>();
    add_service(new IProfile(account_uid));
}

} // namespace Hydra::Horizon::Services::Account
