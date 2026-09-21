#include "core/horizon/services/timesrv/internal/system_clock.hpp"

#include "core/horizon/services/timesrv/internal/steady_clock.hpp"

namespace hydra::horizon::services::timesrv::internal {

SystemClock::SystemClock(SteadyClock& steady_clock_)
    : steady_clock{steady_clock_} {
    updateOffset();
}

u64 SystemClock::updateOffset() {
    steady_time_point = steady_clock.getTimePoint();
    offset_s = static_cast<u64>(
                   std::chrono::duration_cast<std::chrono::seconds>(
                       std::chrono::system_clock::now().time_since_epoch())
                       .count()) -
               steady_time_point;
    return offset_s;
}

u64 SystemClock::getTimePoint() const {
    return offset_s + steady_clock.getTimePoint();
}

} // namespace hydra::horizon::services::timesrv::internal
