#pragma once

#include "core/horizon/services/const.hpp"

namespace Hydra::Horizon::Services::ViSrv {

class IManagerRootService : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetDisplayService(add_service_fn_t add_service);
    result_t
    GetDisplayServiceWithProxyNameExchange(add_service_fn_t add_service);
};

} // namespace Hydra::Horizon::Services::ViSrv
