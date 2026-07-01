#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::psm {

class IPsmSession : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    Handle event_handle{
        INVALID_HANDLE}; // TODO: storing the handle ID is definitely wrong

    bool signal_on_charger_type_change{false};
    bool signal_on_power_supply_change{false};
    bool signal_on_battery_voltage_state_change{false};

    // Commands
    result_t BindStateChangeEvent(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle);
    result_t UnbindStateChangeEvent(kernel::Process* process);
    result_t SetChargerTypeChangeEventEnabled(bool enabled);
    result_t SetPowerSupplyChangeEventEnabled(bool enabled);
    result_t SetBatteryVoltageStateChangeEventEnabled(bool enabled);
};

} // namespace hydra::horizon::services::psm
