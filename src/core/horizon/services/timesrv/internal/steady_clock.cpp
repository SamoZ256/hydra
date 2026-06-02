#include "core/horizon/services/timesrv/internal/steady_clock.hpp"

#include "core/hw/wall_clock.hpp"

namespace hydra::horizon::services::timesrv::internal {

namespace {

constexpr u64 STEADY_CLOCK_OFFSET =
    1'000'000'000; // Offset steady clock by 1s to prevent the offset to go into
                   // negative numbers

}

SteadyClock::SteadyClock(hw::WallClock& wall_clock_) : wall_clock{wall_clock_} {
    UpdateOffset();
}

u64 SteadyClock::GetTimePoint() const {
    return (offset_ns + wall_clock.GetTimeNs()) / 1'000'000'000;
}

u64 SteadyClock::UpdateOffset() {
    offset_ns = STEADY_CLOCK_OFFSET +
                static_cast<u64>(
                    std::chrono::duration_cast<std::chrono::nanoseconds>(
                        std::chrono::steady_clock::now().time_since_epoch())
                        .count()) -
                wall_clock.GetTimeNs();
    return offset_ns;
}

} // namespace hydra::horizon::services::timesrv::internal
