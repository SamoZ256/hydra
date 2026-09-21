#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::mii {

class IDatabaseService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    STUB_REQUEST_COMMAND(setInterfaceVersion); // 5.0.0+
};

} // namespace hydra::horizon::services::mii
