#include "core/horizon/services/ts/measurement_server.hpp"

namespace hydra::horizon::services::ts {

DEFINE_SERVICE_COMMAND_TABLE(IMeasurementServer, 1, GetTemperature)

result_t IMeasurementServer::GetTemperature(Location location,
                                            i32* out_temperature_celsius) {
    LOG_DEBUG(Services, "Location: {}", location);

    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_temperature_celsius = 30;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ts
