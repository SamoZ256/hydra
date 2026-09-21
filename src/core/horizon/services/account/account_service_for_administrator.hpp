#pragma once

#include "core/horizon/services/account/account_service.hpp"

namespace hydra::horizon::services::account {

class IAccountServiceForAdministrator : public IAccountService {
  public:
    IAccountServiceForAdministrator()
        : IAccountService(AccountServiceType::Administrator) {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
};

} // namespace hydra::horizon::services::account
