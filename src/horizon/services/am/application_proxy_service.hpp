#pragma once

#include "horizon/services/service.hpp"

namespace Hydra::Horizon::Services::Am {

class IApplicationProxyService : public ServiceBase {
  public:
  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
};

} // namespace Hydra::Horizon::Services::Am
