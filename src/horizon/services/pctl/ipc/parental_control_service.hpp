#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Pctl::Ipc {

class IParentalControlService : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IParentalControlService)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    // Commands
};

} // namespace Hydra::Horizon::Services::Pctl::Ipc
