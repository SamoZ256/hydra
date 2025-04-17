#pragma once

#include "core/horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::ViSrv {

class IApplicationRootService : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IApplicationRootService)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    // Commands
    void GetDisplayService(REQUEST_COMMAND_PARAMS);
    void GetDisplayServiceWithProxyNameExchange(REQUEST_COMMAND_PARAMS);
};

} // namespace Hydra::Horizon::Services::ViSrv
