#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Am {

class IApmManager : public ServiceBase {
  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
};

} // namespace Hydra::Horizon::Services::Am
