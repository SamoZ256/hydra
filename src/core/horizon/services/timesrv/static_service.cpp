#include "core/horizon/services/timesrv/static_service.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/timesrv/steady_clock.hpp"
#include "core/horizon/services/timesrv/system_clock.hpp"
#include "core/horizon/services/timesrv/time_zone_service.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(IStaticService, 0, GetStandardUserSystemClock, 1,
                             GetStandardNetworkSystemClock, 2,
                             GetStandardSteadyClock, 3, GetTimeZoneService, 4,
                             GetStandardLocalSystemClock, 5,
                             GetEphemeralNetworkSystemClock, 20,
                             GetSharedMemoryNativeHandle, 300,
                             CalculateMonotonicSystemClockBaseTimePoint)

result_t IStaticService::GetStandardUserSystemClock(RequestContext* ctx) {
    AddService(*ctx, new ISystemClock(SystemClockType::StandardUser));
    return RESULT_SUCCESS;
}

result_t IStaticService::GetStandardNetworkSystemClock(RequestContext* ctx) {
    AddService(*ctx, new ISystemClock(SystemClockType::StandardNetwork));
    return RESULT_SUCCESS;
}

result_t IStaticService::GetStandardSteadyClock(RequestContext* ctx) {
    AddService(*ctx, new ISteadyClock());
    return RESULT_SUCCESS;
}

result_t IStaticService::GetTimeZoneService(RequestContext* ctx) {
    AddService(*ctx, new ITimeZoneService());
    return RESULT_SUCCESS;
}

result_t IStaticService::GetStandardLocalSystemClock(RequestContext* ctx) {
    AddService(*ctx, new ISystemClock(SystemClockType::StandardLocal));
    return RESULT_SUCCESS;
}

result_t IStaticService::GetEphemeralNetworkSystemClock(RequestContext* ctx) {
    AddService(*ctx, new ISystemClock(SystemClockType::EphemeralNetwork));
    return RESULT_SUCCESS;
}

result_t IStaticService::GetSharedMemoryNativeHandle(
    System* system, kernel::Process* process,
    OutHandle<HandleAttr::Copy> out_handle) {
    out_handle =
        process->AddHandle(system->GetOS().GetTimeManager().GetSharedMemory());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
