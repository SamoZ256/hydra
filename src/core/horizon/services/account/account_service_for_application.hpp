#pragma once

#include "core/horizon/services/account/account_service.hpp"

namespace hydra::horizon::services::account {

class IAccountServiceForApplication : public IAccountService {
  public:
    IAccountServiceForApplication()
        : IAccountService(AccountServiceType::Application) {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(initializeApplicationInfoV0);
    result_t getBaasAccountManagerForApplication(RequestContext* ctx,
                                                 uuid_t user_id);
    STUB_REQUEST_COMMAND(storeSaveDataThumbnail);
    STUB_REQUEST_COMMAND(initializeApplicationInfo);
    STUB_REQUEST_COMMAND(isUserAccountSwitchLocked);
    STUB_REQUEST_COMMAND(initializeApplicationInfoV2);
};

} // namespace hydra::horizon::services::account
