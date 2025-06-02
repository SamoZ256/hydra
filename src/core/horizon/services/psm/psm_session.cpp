#include "core/horizon/services/psm/psm_session.hpp"

#include "core/horizon/kernel/kernel.hpp"

// TODO: actually signal the handle

namespace hydra::horizon::services::psm {

DEFINE_SERVICE_COMMAND_TABLE(IPsmSession, 0, BindStateChangeEvent, 1,
                             UnbindStateChangeEvent, 2,
                             SetChargerTypeChangeEventEnabled, 3,
                             SetPowerSupplyChangeEventEnabled, 4,
                             SetBatteryVoltageStateChangeEventEnabled)

result_t
IPsmSession::BindStateChangeEvent(OutHandle<HandleAttr::Copy> out_handle) {
    event_handle_id = KERNEL_INSTANCE.AddHandle(new kernel::Event());
    out_handle = event_handle_id;
    return RESULT_SUCCESS;
}

result_t IPsmSession::UnbindStateChangeEvent() {
    KERNEL_INSTANCE.FreeHandle(event_handle_id);
    event_handle_id = INVALID_HANDLE_ID;
    return RESULT_SUCCESS;
}

result_t IPsmSession::SetChargerTypeChangeEventEnabled(bool enabled) {
    signal_on_charger_type_change = enabled;
    return RESULT_SUCCESS;
}

result_t IPsmSession::SetPowerSupplyChangeEventEnabled(bool enabled) {
    signal_on_power_supply_change = enabled;
    return RESULT_SUCCESS;
}

result_t IPsmSession::SetBatteryVoltageStateChangeEventEnabled(bool enabled) {
    signal_on_battery_voltage_state_change = enabled;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::psm
