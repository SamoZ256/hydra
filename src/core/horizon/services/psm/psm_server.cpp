#include "core/horizon/services/psm/psm_server.hpp"

#include "core/horizon/services/psm/psm_session.hpp"

// TODO: report real data on devices with batteries + make this configurable

namespace hydra::horizon::services::psm {

DEFINE_SERVICE_COMMAND_TABLE(IPsmServer, 0, getBatteryChargePercentage, 1,
                             getChargerType, 7, openSession, 13,
                             getRawBatteryChargePercentage, 14,
                             isEnoughPowerSupplied, 15, getBatteryAgePercentage,
                             17, getBatteryChargeInfoFields)

result_t IPsmServer::getBatteryChargePercentage(u32* out_percentage) {
    *out_percentage = 100;
    return RESULT_SUCCESS;
}

result_t IPsmServer::getChargerType(ChargerType* out_type) {
    *out_type = ChargerType::EnoughPower;
    return RESULT_SUCCESS;
}

result_t IPsmServer::openSession(RequestContext* ctx) {
    addService(*ctx, new IPsmSession());
    return RESULT_SUCCESS;
}

result_t IPsmServer::getRawBatteryChargePercentage(f64* out_percentage) {
    *out_percentage = 100.0;
    return RESULT_SUCCESS;
}

result_t IPsmServer::isEnoughPowerSupplied(bool* out_is_enough) {
    *out_is_enough = true;
    return RESULT_SUCCESS;
}

result_t IPsmServer::getBatteryAgePercentage(f64* out_percentage) {
    *out_percentage = 0.0;
    return RESULT_SUCCESS;
}

result_t
IPsmServer::getBatteryChargeInfoFields(BatteryChargeInfoFields* out_fields) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_fields = {};
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::psm
