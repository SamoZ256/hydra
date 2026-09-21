#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::visrv {

class IApplicationRootService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getDisplayService(RequestContext* ctx);
    result_t getDisplayServiceWithProxyNameExchange(RequestContext* ctx);
};

} // namespace hydra::horizon::services::visrv
