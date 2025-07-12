#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::psm {

class IPsmSession : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    handle_id_t event_handle_id{
        INVALID_HANDLE_ID}; // TODO: storing the handle ID is definitely wrong

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
