#include "core/horizon/services/psm/psm_session.hpp"

#include "core/horizon/kernel/process.hpp"

// TODO: actually signal the handle

namespace hydra::horizon::services::psm {

DEFINE_SERVICE_COMMAND_TABLE(IPsmSession, 0, bindStateChangeEvent, 1,
                             unbindStateChangeEvent, 2,
                             setChargerTypeChangeEventEnabled, 3,
                             setPowerSupplyChangeEventEnabled, 4,
                             setBatteryVoltageStateChangeEventEnabled)

result_t
IPsmSession::bindStateChangeEvent(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle) {
    event_handle = process->addHandle(
        new kernel::Event(false, "IPsmSession state change event"));
    out_handle = event_handle;
    return RESULT_SUCCESS;
}

result_t IPsmSession::unbindStateChangeEvent(kernel::Process* process) {
    process->freeHandle(event_handle);
    event_handle = INVALID_HANDLE;
    return RESULT_SUCCESS;
}

result_t IPsmSession::setChargerTypeChangeEventEnabled(bool enabled) {
    signal_on_charger_type_change = enabled;
    return RESULT_SUCCESS;
}

result_t IPsmSession::setPowerSupplyChangeEventEnabled(bool enabled) {
    signal_on_power_supply_change = enabled;
    return RESULT_SUCCESS;
}

result_t IPsmSession::setBatteryVoltageStateChangeEventEnabled(bool enabled) {
    signal_on_battery_voltage_state_change = enabled;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::psm
