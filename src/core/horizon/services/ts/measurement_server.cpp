#include "core/horizon/services/ts/measurement_server.hpp"

namespace hydra::horizon::services::ts {

DEFINE_SERVICE_COMMAND_TABLE(IMeasurementServer, 1, getTemperature)

result_t IMeasurementServer::getTemperature(Location location,
                                            i32* out_temperature_celsius) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "location: {}", location);

    // HACK
    *out_temperature_celsius = 30;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ts
