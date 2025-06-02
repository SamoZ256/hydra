#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::psm {

enum class ChargerType {
    Unconnected,
    EnoughPower,
    NoPower,
    NotSupported,
};

class IPsmServer : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetBatteryChargePercentage(u32* out_percentage);
    result_t GetChargerType(ChargerType* out_type);
    result_t OpenSession(add_service_fn_t add_service);
    result_t GetRawBatteryChargePercentage(f64* out_percentage);
    result_t IsEnoughPowerSupplied(bool* out_is_enough);
    result_t GetBatteryAgePercentage(f64* out_percentage);
};

} // namespace hydra::horizon::services::psm
