#pragma once

#include "core/horizon/services/nfp/nfp.hpp"

namespace hydra::horizon::services::nfp {

class IUser : public INfp {
  public:
    IUser() : INfp(PermissionLevel::User) {}

  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;
};

} // namespace hydra::horizon::services::nfp
