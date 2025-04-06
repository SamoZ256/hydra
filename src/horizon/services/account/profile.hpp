#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Account {

class IProfile : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IProfile)

    IProfile(u128 account_uid_) : account_uid{account_uid_} {}

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    u128 account_uid;

    // Commands
    void Get(REQUEST_COMMAND_PARAMS);
};

} // namespace Hydra::Horizon::Services::Account
