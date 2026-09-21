#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::socket::resolver {

class IResolver : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getAddrInfoRequest();
};

} // namespace hydra::horizon::services::socket::resolver
