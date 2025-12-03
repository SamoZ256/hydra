#include "core/horizon/services/timesrv/time_zone_service.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(ITimeZoneService, 0, GetDeviceLocationName, 4,
                             LoadTimeZoneRule, 100, ToCalendarTime, 101,
                             ToCalendarTimeWithMyRule, 201, ToPosixTime, 202,
                             ToPosixTimeWithMyRule)

result_t ITimeZoneService::GetDeviceLocationName(LocationName* out_name) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    std::memcpy(out_name, "UTC\0", 4);
    return RESULT_SUCCESS;
}

result_t ITimeZoneService::LoadTimeZoneRule(
    LocationName location_name,
    OutBuffer<BufferAttr::MapAlias> out_rule_buffer) {
    LOG_FUNC_STUBBED(Services);

    LOG_DEBUG(Services, "Location: {}", location_name.name);

    // HACK
    out_rule_buffer.writer->Write(TimeZoneRule{});
    return RESULT_SUCCESS;
}

result_t
ITimeZoneService::ToCalendarTime(i64 posix_time,
                                 InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                                 ToCalendarTimeWithMyRuleOut* out) {
    return ToCalendarTimeImpl(posix_time,
                              in_rule_buffer.reader->Read<TimeZoneRule>(),
                              out->time, out->additional_info);
}

result_t
ITimeZoneService::ToCalendarTimeWithMyRule(i64 posix_time,
                                           ToCalendarTimeWithMyRuleOut* out) {
    // TODO: my rule (probably the current timezone rule?)
    return ToCalendarTimeImpl(posix_time, {}, out->time, out->additional_info);
}

result_t ITimeZoneService::ToPosixTime(
    CalendarTime calendar_time, InBuffer<BufferAttr::MapAlias> in_rule_buffer,
    i32* out_count, OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    i64 time;
    const auto res = ToPosixTimeImpl(
        calendar_time, in_rule_buffer.reader->Read<TimeZoneRule>(), time);

    out_buffer.writer->Write(time);
    *out_count = static_cast<i32>(out_buffer.writer->Tell() / sizeof(i64));
    return res;
}

result_t ITimeZoneService::ToPosixTimeWithMyRule(
    CalendarTime calendar_time, i32* out_count,
    OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    i64 time;
    // TODO: my rule (probably the current timezone rule?)
    const auto res = ToPosixTimeImpl(calendar_time, {}, time);

    out_buffer.writer->Write(time);
    *out_count = static_cast<i32>(out_buffer.writer->Tell() / sizeof(i64));
    return res;
}

result_t ITimeZoneService::ToCalendarTimeImpl(
    i64 posix_time, const TimeZoneRule& rule, CalendarTime& out_time,
    CalendarAdditionalInfo& out_additional_info) {
    LOG_FUNC_STUBBED(Services);

    // Time
    // TODO

    // Additional info
    out_additional_info = {
        .day_of_week = 0,           // TODO
        .day_of_year = 0,           // TODO
        .timezone_name = "UTC"_u64, // HACK
        .dst = 0,                   // TODO
        .seconds_rel_to_utc = 0,    // TODO
    };

    return RESULT_SUCCESS;
}

result_t ITimeZoneService::ToPosixTimeImpl(const CalendarTime& calendar_time,
                                           const TimeZoneRule& rule,
                                           i64& out_time) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    out_time = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
