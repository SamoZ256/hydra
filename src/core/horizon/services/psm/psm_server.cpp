#include "core/horizon/services/psm/psm_server.hpp"

#include "core/horizon/services/psm/psm_session.hpp"

// TODO: report real data on devices with batteries + make this configurable

namespace hydra::horizon::services::psm {

DEFINE_SERVICE_COMMAND_TABLE(IPsmServer, 0, GetBatteryChargePercentage, 1,
                             GetChargerType, 7, OpenSession, 13,
                             GetRawBatteryChargePercentage, 14,
                             IsEnoughPowerSupplied, 15, GetBatteryAgePercentage)

result_t IPsmServer::GetBatteryChargePercentage(u32* out_percentage) {
    *out_percentage = 100;
    return RESULT_SUCCESS;
}

result_t IPsmServer::GetChargerType(ChargerType* out_type) {
    *out_type = ChargerType::EnoughPower;
    return RESULT_SUCCESS;
}

result_t IPsmServer::OpenSession(add_service_fn_t add_service) {
    add_service(new IPsmSession());
    return RESULT_SUCCESS;
}

result_t IPsmServer::GetRawBatteryChargePercentage(f64* out_percentage) {
    *out_percentage = 100.0;
    return RESULT_SUCCESS;
}

result_t IPsmServer::IsEnoughPowerSupplied(bool* out_is_enough) {
    *out_is_enough = true;
    return RESULT_SUCCESS;
}

result_t IPsmServer::GetBatteryAgePercentage(f64* out_percentage) {
    *out_percentage = 0.0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::psm
