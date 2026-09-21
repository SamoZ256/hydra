#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::friends {

class IFriendService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(updateUserPresence);
};

} // namespace hydra::horizon::services::friends
