#pragma once

#include "horizon/services/service.hpp"

namespace Hydra::Horizon::Services {

class ServiceManager : public ServiceBase {
  public:
  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
};

} // namespace Hydra::Horizon::Services
