#include "core/horizon/services/timesrv/static_service.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/timesrv/steady_clock.hpp"
#include "core/horizon/services/timesrv/system_clock.hpp"
#include "core/horizon/services/timesrv/time_zone_service.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(IStaticService, 0, getStandardUserSystemClock, 1,
                             getStandardNetworkSystemClock, 2,
                             getStandardSteadyClock, 3, getTimeZoneService, 4,
                             getStandardLocalSystemClock, 5,
                             getEphemeralNetworkSystemClock, 20,
                             getSharedMemoryNativeHandle, 300,
                             calculateMonotonicSystemClockBaseTimePoint)

result_t IStaticService::getStandardUserSystemClock(RequestContext* ctx) {
    addService(*ctx, new ISystemClock(SystemClockType::StandardUser));
    return RESULT_SUCCESS;
}

result_t IStaticService::getStandardNetworkSystemClock(RequestContext* ctx) {
    addService(*ctx, new ISystemClock(SystemClockType::StandardNetwork));
    return RESULT_SUCCESS;
}

result_t IStaticService::getStandardSteadyClock(RequestContext* ctx) {
    addService(*ctx, new ISteadyClock());
    return RESULT_SUCCESS;
}

result_t IStaticService::getTimeZoneService(RequestContext* ctx) {
    addService(*ctx, new ITimeZoneService());
    return RESULT_SUCCESS;
}

result_t IStaticService::getStandardLocalSystemClock(RequestContext* ctx) {
    addService(*ctx, new ISystemClock(SystemClockType::StandardLocal));
    return RESULT_SUCCESS;
}

result_t IStaticService::getEphemeralNetworkSystemClock(RequestContext* ctx) {
    addService(*ctx, new ISystemClock(SystemClockType::EphemeralNetwork));
    return RESULT_SUCCESS;
}

result_t IStaticService::getSharedMemoryNativeHandle(
    System* system, kernel::Process* process,
    OutHandle<HandleAttr::Copy> out_handle) {
    out_handle =
        process->addHandle(system->getOs().getTimeManager().getSharedMemory());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
