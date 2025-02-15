#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::HosBinder {

class IHOSBinderDriver : public ServiceBase {
  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    void AdjustRefcount(REQUEST_COMMAND_PARAMS);
};

} // namespace Hydra::Horizon::Services::HosBinder
