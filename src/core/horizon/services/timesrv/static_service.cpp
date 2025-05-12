#include "core/horizon/services/timesrv/static_service.hpp"

#include "core/horizon/services/timesrv/steady_clock.hpp"
#include "core/horizon/services/timesrv/system_clock.hpp"
#include "core/horizon/services/timesrv/time_zone_service.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(IStaticService, 0, GetStandardUserSystemClock, 1,
                             GetStandardNetworkSystemClock, 2,
                             GetStandardSteadyClock, 3, GetTimeZoneService, 4,
                             GetStandardLocalSystemClock, 5,
                             GetEphemeralNetworkSystemClock)

result_t
IStaticService::GetStandardUserSystemClock(add_service_fn_t add_service) {
    add_service(new ISystemClock(SystemClockType::StandardUser));
    return RESULT_SUCCESS;
}

result_t
IStaticService::GetStandardNetworkSystemClock(add_service_fn_t add_service) {
    add_service(new ISystemClock(SystemClockType::StandardNetwork));
    return RESULT_SUCCESS;
}

result_t IStaticService::GetStandardSteadyClock(add_service_fn_t add_service) {
    add_service(new ISteadyClock());
    return RESULT_SUCCESS;
}

result_t IStaticService::GetTimeZoneService(add_service_fn_t add_service) {
    add_service(new ITimeZoneService());
    return RESULT_SUCCESS;
}

result_t
IStaticService::GetStandardLocalSystemClock(add_service_fn_t add_service) {
    add_service(new ISystemClock(SystemClockType::StandardLocal));
    return RESULT_SUCCESS;
}

result_t
IStaticService::GetEphemeralNetworkSystemClock(add_service_fn_t add_service) {
    add_service(new ISystemClock(SystemClockType::EphemeralNetwork));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
