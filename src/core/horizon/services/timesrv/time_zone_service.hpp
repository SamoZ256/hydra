#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::timesrv {

struct LocationName {
    char name[0x24];
};

struct TimeZoneRule {
    u8 data[0x4000];
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

class ITimeZoneService : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetDeviceLocationName(LocationName* out_name);
    result_t LoadTimeZoneRule(LocationName location_name,
                              OutBuffer<BufferAttr::MapAlias> out_rule_buffer);
    result_t ToCalendarTime(u64 posix_time,
                            InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                            ToCalendarTimeWithMyRuleOut* out);
    result_t ToCalendarTimeWithMyRule(u64 posix_time,
                                      ToCalendarTimeWithMyRuleOut* out);

    result_t ToCalendarTimeImpl(i64 posix_time, const TimeZoneRule& rule,
                                CalendarTime& out_time,
                                CalendarAdditionalInfo& out_additional_info);
};

} // namespace hydra::horizon::services::timesrv
