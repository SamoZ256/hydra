#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IDebugFunctions : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
};

} // namespace hydra::horizon::services::am
