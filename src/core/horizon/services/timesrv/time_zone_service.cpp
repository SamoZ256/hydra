#include "core/horizon/services/timesrv/time_zone_service.hpp"

#include "core/system.hpp"

namespace hydra::horizon::services::timesrv {

DEFINE_SERVICE_COMMAND_TABLE(ITimeZoneService, 0, GetDeviceLocationName, 4,
                             LoadTimeZoneRule, 100, ToCalendarTime, 101,
                             ToCalendarTimeWithMyRule, 201, ToPosixTime, 202,
                             ToPosixTimeWithMyRule)

result_t ITimeZoneService::GetDeviceLocationName(RequestContext* ctx,
                                                 LocationName* out_name) {
    const auto name = ctx->system.GetOS()
                          .GetTimeManager()
                          .GetTimeZoneManager()
                          .GetDeviceLocationName();
    std::memcpy(out_name->name, name.data(), name.size());
    out_name->name[name.size()] = '\0';
    return RESULT_SUCCESS;
}

result_t ITimeZoneService::LoadTimeZoneRule(
    RequestContext* ctx, LocationName location_name,
    OutBuffer<BufferAttr::MapAlias> out_rule_buffer) {
    TimeZoneRule rule;
    ctx->system.GetOS().GetTimeManager().GetTimeZoneManager().LoadRule(
        location_name.name, rule);

    out_rule_buffer.stream->Write(rule);
    return RESULT_SUCCESS;
}

result_t
ITimeZoneService::ToCalendarTime(i64 posix_time,
                                 InBuffer<BufferAttr::MapAlias> in_rule_buffer,
                                 ToCalendarTimeWithMyRuleOut* out) {
    return ToCalendarTimeImpl(posix_time,
                              in_rule_buffer.stream->Read<TimeZoneRule>(),
                              out->time, out->additional_info);
}

result_t
ITimeZoneService::ToCalendarTimeWithMyRule(RequestContext* ctx, i64 posix_time,
                                           ToCalendarTimeWithMyRuleOut* out) {
    return ToCalendarTimeImpl(
        posix_time,
        ctx->system.GetOS().GetTimeManager().GetTimeZoneManager().GetMyRule(),
        out->time, out->additional_info);
}

result_t ITimeZoneService::ToPosixTime(
    CalendarTime calendar_time, InBuffer<BufferAttr::MapAlias> in_rule_buffer,
    i32* out_count, OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    i64 time;
    const auto res = ToPosixTimeImpl(
        calendar_time, in_rule_buffer.stream->Read<TimeZoneRule>(), time);

    out_buffer.stream->Write(time);
    *out_count = static_cast<i32>(out_buffer.stream->GetSeek() / sizeof(i64));
    return res;
}

result_t ITimeZoneService::ToPosixTimeWithMyRule(
    RequestContext* ctx, CalendarTime calendar_time, i32* out_count,
    OutBuffer<BufferAttr::HipcPointer> out_buffer) {
    i64 time;
    const auto res = ToPosixTimeImpl(
        calendar_time,
        ctx->system.GetOS().GetTimeManager().GetTimeZoneManager().GetMyRule(),
        time);

    out_buffer.stream->Write(time);
    *out_count = static_cast<i32>(out_buffer.stream->GetSeek() / sizeof(i64));
    return res;
}

result_t ITimeZoneService::ToCalendarTimeImpl(
    i64 posix_time, const TimeZoneRule& rule, CalendarTime& out_time,
    CalendarAdditionalInfo& out_additional_info) {
    // Find the type
    u32 type_idx = rule.default_type;
    for (u32 i = 0; i < rule.time_count; ++i) {
        if (posix_time >= rule.ats[i]) {
            type_idx = rule.type_indices[i];
        } else {
            break;
        }
    }

    const auto& info = rule.type_infos[type_idx];

    // Adjust by GMT offset
    auto adjusted_tp = std::chrono::sys_seconds{
        std::chrono::seconds{posix_time + info.gmt_offset}};
    auto days = std::chrono::floor<std::chrono::days>(adjusted_tp);
    std::chrono::year_month_day ymd{days};
    std::chrono::hh_mm_ss hms{adjusted_tp - days};

    // Get time zone name
    const char* tz_name = rule.chars + info.abbreviation_list_index;
    if (tz_name == nullptr) {
        tz_name = "UTC";
    }

    // Output
    out_time = {
        .year = static_cast<u16>(static_cast<int>(ymd.year())),
        .month = static_cast<u8>(ymd.month().operator unsigned()),
        .day = static_cast<u8>(ymd.day().operator unsigned()),
        .hour = static_cast<u8>(hms.hours().count()),
        .minute = static_cast<u8>(hms.minutes().count()),
        .second = static_cast<u8>(hms.seconds().count()),
    };

    out_additional_info = {
        .day_of_week = static_cast<u8>(
            std::chrono::year_month_weekday{ymd}.weekday().c_encoding()),
        .day_of_year =
            static_cast<u32>((std::chrono::sys_days{ymd} -
                              std::chrono::sys_days{ymd.year() / 1 / 0})
                                 .count()),
        .timezone_name = ToU64String(tz_name),
        .dst = info.is_day_saving_time ? 1u : 0u,
        .seconds_rel_to_utc = info.gmt_offset,
    };

    return RESULT_SUCCESS;
}

result_t ITimeZoneService::ToPosixTimeImpl(const CalendarTime& calendar_time,
                                           const TimeZoneRule& rule,
                                           i64& out_time) {
    (void)rule;

    LOG_FUNC_WITH_ARGS_STUBBED(Services, "calendar time: {}", calendar_time);

    // HACK
    out_time = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::timesrv
