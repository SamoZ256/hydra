#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/timesrv/const.hpp"

namespace hydra::horizon::services::timesrv {

struct LocationName {
    char name[0x24];
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
    result_t GetDeviceLocationName(RequestContext* ctx, LocationName* out_name);
    result_t LoadTimeZoneRule(RequestContext* ctx, LocationName location_name,
                              OutBuffer<BufferAttr::MapAlias> out_rule_buffer);
    result_t ToCalendarTime(i64 posix_time,
                            InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                            ToCalendarTimeWithMyRuleOut* out);
    result_t ToCalendarTimeWithMyRule(RequestContext* ctx, i64 posix_time,
                                      ToCalendarTimeWithMyRuleOut* out);
    result_t ToPosixTime(CalendarTime calendar_time,
                         InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                         i32* out_count,
                         OutBuffer<BufferAttr::HipcPointer> out_buffer);
    result_t
    ToPosixTimeWithMyRule(RequestContext* ctx, CalendarTime calendar_time,
                          i32* out_count,
                          OutBuffer<BufferAttr::HipcPointer> out_buffer);

    // Impl
    result_t ToCalendarTimeImpl(i64 posix_time, const TimeZoneRule& rule,
                                CalendarTime& out_time,
                                CalendarAdditionalInfo& out_additional_info);
    // TODO: support more than 1 time?
    result_t ToPosixTimeImpl(const CalendarTime& calendar_time,
                             const TimeZoneRule& rule, i64& out_time);
};

} // namespace hydra::horizon::services::timesrv

ENABLE_STRUCT_FORMATTING(hydra::horizon::services::timesrv::CalendarTime, year,
                         "", "year", month, "", "month", day, "", "day", hour,
                         "", "hour", minute, "", "minute", second, "", "second")
