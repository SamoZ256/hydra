#pragma once
#include "core/horizon/services/account/account_service.hpp"

namespace hydra::horizon::services::account {

class IAccountServiceForSystemService : public IAccountService {
  public:
    IAccountServiceForSystemService()
        : IAccountService(AccountServiceType::SystemService) {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    // TODO: correct?
    result_t getUserRegistrationNotifier(RequestContext* ctx);
    // TODO: correct?
    result_t getUserStateChangeNotifier(RequestContext* ctx);
    // TODO: correct?
    result_t getBaasAccountManagerForSystemService(RequestContext* ctx);
};

} // namespace hydra::horizon::services::account
