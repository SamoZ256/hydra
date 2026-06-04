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

struct TimeTypeInfo {
    i32 gmt_offset;
    bool is_day_saving_time;
    u8 _padding_x5[3];
    i32 abbreviation_list_index;
    bool is_standard_time_daylight;
    bool is_gmt;
    u8 _padding_xe[2];
};

struct TimeZoneRule {
    static constexpr usize MAX_TYPE_COUNT = 128;
    static constexpr usize MAX_CHAR_LEN = 50;
    static constexpr usize MAX_LEAP_LEN = 50;
    static constexpr usize MAX_TIME_COUNT = 1000;
    static constexpr usize MAX_NAME_LEN = 255;
    static constexpr usize CHAR_ARRAY_SIZE = 2 * (MAX_NAME_LEN + 1);

    i32 time_len;
    i32 type_len;
    i32 char_len;
    bool go_back;
    bool go_ahead;
    i64 ats[MAX_TIME_COUNT];
    u8 type_indices[MAX_TIME_COUNT];
    TimeTypeInfo type_infos[MAX_TYPE_COUNT];
    char chars[CHAR_ARRAY_SIZE];
    i32 default_type;
    u8 _padding_x2d40[0x12c0];
};
#pragma pack(pop)

} // namespace hydra::horizon::services::timesrv
