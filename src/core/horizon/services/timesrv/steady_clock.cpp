#include "core/horizon/services/timesrv/steady_clock.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(ISteadyClock, 0, GetCurrentTimePoint)

result_t
ISteadyClock::GetCurrentTimePoint(RequestContext* ctx,
                                  SteadyClockTimePoint* out_time_point) {
    *out_time_point = {
        .time_point = ctx->system.GetOS()
                          .GetTimeManager()
                          .GetSteadyClock()
                          .GetTimePoint(),
        .clock_source_id = CLOCK_SOURCE_ID,
    };
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
