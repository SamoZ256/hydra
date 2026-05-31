#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::timesrv {

struct LocationName {
    char name[0x24];
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

struct CalendarTime {
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
    u8 _unknown;
};

struct CalendarAdditionalInfo {
    u32 day_of_week; // 0-based
    u32 day_of_year; // 0-based
    u64 timezone_name;
    u32 dst; // TODO: what is this? (0 = no DST, 1 = DST)
    i32 seconds_rel_to_utc;
};

namespace {

struct ToCalendarTimeWithMyRuleOut {
    CalendarTime time;
    CalendarAdditionalInfo additional_info;
};

} // namespace

class ITimeZoneService : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t GetDeviceLocationName(LocationName* out_name);
    result_t LoadTimeZoneRule(LocationName location_name,
                              OutBuffer<BufferAttr::MapAlias> out_rule_buffer);
    result_t ToCalendarTime(i64 posix_time,
                            InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                            ToCalendarTimeWithMyRuleOut* out);
    result_t ToCalendarTimeWithMyRule(i64 posix_time,
                                      ToCalendarTimeWithMyRuleOut* out);
    result_t ToPosixTime(CalendarTime calendar_time,
                         InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                         i32* out_count,
                         OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t
    ToPosixTimeWithMyRule(CalendarTime calendar_time, i32* out_count,
                          OutBuffer<BufferAttr::HipcPointer> out_buffer);

    // Impl
    result_t ToCalendarTimeImpl(i64 posix_time, const TimeZoneRule& rule,
                                CalendarTime& out_time,
                                CalendarAdditionalInfo& out_additional_info);
    // TODO: support more than 1 time?
    result_t ToPosixTimeImpl(const CalendarTime& calendar_time,
                             const TimeZoneRule& rule, i64& out_time);

    // Helpers
    std::string_view GetLocationName();
};

} // namespace hydra::horizon::services::timesrv

ENABLE_STRUCT_FORMATTING(hydra::horizon::services::timesrv::CalendarTime, year,
                         "", "year", month, "", "month", day, "", "day", hour,
                         "", "hour", minute, "", "minute", second, "", "second")
