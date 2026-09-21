#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::pctl {

class IParentalControlServiceFactory : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t createService(RequestContext* ctx);
    result_t createServiceWithoutInitialize(RequestContext* ctx);
};

} // namespace hydra::horizon::services::pctl
