#include "core/horizon/services/timesrv/time_zone_service.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(ITimeZoneService, 0, GetDeviceLocationName, 4,
                             LoadTimeZoneRule, 100, ToCalendarTime, 101,
                             ToCalendarTimeWithMyRule)

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
ITimeZoneService::ToCalendarTime(u64 posix_time,
                                 InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                                 ToCalendarTimeWithMyRuleOut* out) {
    return ToCalendarTimeImpl(posix_time,
                              in_rule_buffer.reader->Read<TimeZoneRule>(),
                              out->time, out->additional_info);
}

result_t
ITimeZoneService::ToCalendarTimeWithMyRule(u64 posix_time,
                                           ToCalendarTimeWithMyRuleOut* out) {
    // TODO: my rule (probably the current timezone rule?)
    return ToCalendarTimeImpl(posix_time, {}, out->time, out->additional_info);
}

result_t ITimeZoneService::ToCalendarTimeImpl(
    i64 posix_time, const TimeZoneRule& rule, CalendarTime& out_time,
    CalendarAdditionalInfo& out_additional_info) {
    LOG_FUNC_STUBBED(Services);

    // Time
    // TODO

    // Additional info
    out_additional_info = {
        .day_of_week = 0,                   // TODO
        .day_of_year = 0,                   // TODO
        .timezone_name = str_to_u64("UTC"), // HACK
        .dst = 0,                           // TODO
        .seconds_rel_to_utc = 0,            // TODO
    };

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
