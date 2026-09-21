#pragma once

#include "core/horizon/services/nfc/nfc.hpp"

namespace hydra::horizon::services::nfc {

class IUser : public INfc {
  public:
    IUser() : INfc(PermissionLevel::User) {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;
};

} // namespace hydra::horizon::services::nfc
