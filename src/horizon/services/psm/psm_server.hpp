#pragma once

#include "horizon/services/service_base.hpp"

namespace Hydra::Horizon::Services::Psm {

class IPsmServer : public ServiceBase {
  public:
    DEFINE_SERVICE_VIRTUAL_FUNCTIONS(IPsmServer)

  protected:
    void RequestImpl(REQUEST_IMPL_PARAMS) override;

  private:
    // Commands
    void GetBatteryChargePercentage(REQUEST_COMMAND_PARAMS);
    void GetChargerType(REQUEST_COMMAND_PARAMS);
    void GetRawBatteryChargePercentage(REQUEST_COMMAND_PARAMS);
    void IsEnoughPowerSupplied(REQUEST_COMMAND_PARAMS);
    void GetBatteryAgePercentage(REQUEST_COMMAND_PARAMS);
};

} // namespace Hydra::Horizon::Services::Psm
