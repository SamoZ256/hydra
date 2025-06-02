#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nifm {

class IStaticService : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t CreateGeneralServiceOld(add_service_fn_t add_service);
    result_t CreateGeneralService(add_service_fn_t add_service);
};

} // namespace hydra::horizon::services::nifm
