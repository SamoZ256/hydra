#include "core/horizon/services/timesrv/internal/system_clock.hpp"

#include "core/horizon/services/timesrv/internal/steady_clock.hpp"

namespace hydra::horizon::services::timesrv::internal {

SystemClock::SystemClock(SteadyClock& steady_clock_)
    : steady_clock{steady_clock_} {
    UpdateOffset();
}

u64 SystemClock::UpdateOffset() {
    steady_time_point = steady_clock.GetTimePoint();
    offset_s = static_cast<u64>(
                   std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count()) -
               steady_time_point;
    return offset_s;
}

u64 SystemClock::GetTimePoint() const {
    return offset_s + steady_clock.GetTimePoint();
}

} // namespace hydra::horizon::services::timesrv::internal
