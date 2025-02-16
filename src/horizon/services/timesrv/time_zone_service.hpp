#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::TimeSrv {

class ITimeZoneService : public ServiceBase {
  public:
    DEFINE_VIRTUAL_CLONE(ITimeZoneService)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
};

} // namespace Hydra::Horizon::Services::TimeSrv
