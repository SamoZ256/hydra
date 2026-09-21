#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::psc {

class IPmService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getPmModule(RequestContext* ctx);
};

} // namespace hydra::horizon::services::psc
