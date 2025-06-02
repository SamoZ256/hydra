#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nfc {

class IUserManager : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t CreateUserInterface(add_service_fn_t add_service);
};

} // namespace hydra::horizon::services::nfc
