#include "core/horizon/services/timesrv/system_clock.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(ISystemClock, 0, GetCurrentTime, 2,
                             GetSystemClockContext)

result_t ISystemClock::GetCurrentTime(i64* out_posix_time) {
    // TODO: take type into account
    *out_posix_time = std::chrono::duration_cast<std::chrono::seconds>(
                          std::chrono::system_clock::now().time_since_epoch())
                          .count();
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
