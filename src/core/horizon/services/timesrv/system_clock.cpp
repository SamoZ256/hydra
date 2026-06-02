#include "core/horizon/services/timesrv/system_clock.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(ISystemClock, 0, GetCurrentTime, 2,
                             GetSystemClockContext)

result_t ISystemClock::GetCurrentTime(RequestContext* ctx,
                                      i64* out_posix_time) {
    // TODO: take type into account?
    (void)type;
    *out_posix_time = static_cast<i64>(
        ctx->system.GetOS().GetTimeManager().GetSystemClock().GetTimePoint());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
