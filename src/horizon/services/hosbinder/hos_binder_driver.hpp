#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::HosBinder {

class IHOSBinderDriver : public ServiceBase {
  public:
  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
};

} // namespace Hydra::Horizon::Services::HosBinder
