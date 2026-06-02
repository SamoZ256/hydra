#pragma once

namespace hydra::horizon::services::timesrv {

// TODO: use dynamic IDs
constexpr u128 CLOCK_SOURCE_ID =
    (static_cast<u128>(0x3901f6dde03fe1c0ull) << 64) |
    static_cast<u128>(0x4950593b2204f44dull);

#pragma pack(push, 1)
struct SteadyClockTimePoint {
    u64 time_point;
    u128 clock_source_id;
};

struct SteadyClockContext {
    u64 internal_offset;
    u128 clock_source_id;
};

struct SystemClockContext {
    i64 offset;
    SteadyClockTimePoint steady_time_point;
};

struct ContinuousAdjustmentTimePoint {
    u64 clock_offset;
    i64 multiplier;
    i64 divisor_log2;
    SystemClockContext context;
};
#pragma pack(pop)

} // namespace hydra::horizon::services::timesrv
