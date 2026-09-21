#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nifm {

class IStaticService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t createGeneralServiceOld(RequestContext* ctx);
    result_t createGeneralService(RequestContext* ctx);
};

} // namespace hydra::horizon::services::nifm
