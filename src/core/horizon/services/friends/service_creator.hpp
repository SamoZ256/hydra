#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::friends {

class IServiceCreator : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t createFriendService(RequestContext* ctx);
    result_t createNotificationService(RequestContext* ctx,
                                       uuid_t user_id); // 2.0.0+
};

} // namespace hydra::horizon::services::friends
