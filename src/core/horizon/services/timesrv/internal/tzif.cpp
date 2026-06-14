#include "core/horizon/services/timesrv/internal/tzif.hpp"

#include <bit>

namespace hydra::horizon::services::timesrv::internal {

namespace {

const int YEARS_PER_REPEAT = 400;
const long AVERAGE_SECONDS_PER_YEAR = 31556952;
const long SECONDS_PER_REPEAT = YEARS_PER_REPEAT * AVERAGE_SECONDS_PER_YEAR;

struct TzifHeader {
    u32 magic;
    u8 version;
    u8 _reserved[15];
    u32 ttis_gmt_count;
    u32 ttis_std_count;
    u32 leap_count;
    u32 time_count;
    u32 type_count;
    u32 char_count;
};

template <typename T>
static T Decode(T value) {
    if constexpr (std::endian::native == std::endian::little) {
        return std::byteswap(value);
    } else {
        return value;
    }
}

static bool DifferByRepeat(i64 t1, i64 t0) {
    return (t1 - t0) == SECONDS_PER_REPEAT;
}

static bool TimeTypeEquals(const TimeZoneRule& rule, u8 a_index, u8 b_index) {
    if (a_index < 0 || a_index >= rule.type_count || b_index < 0 ||
        b_index >= rule.type_count) {
        return false;
    }

    TimeTypeInfo a = rule.type_infos[a_index];
    TimeTypeInfo b = rule.type_infos[b_index];

    return a.gmt_offset == b.gmt_offset &&
           a.is_day_saving_time == b.is_day_saving_time &&
           a.is_standard_time_daylight == b.is_standard_time_daylight &&
           a.is_gmt == b.is_gmt &&
           std::string_view(rule.chars + a.abbreviation_list_index) ==
               std::string_view(rule.chars + b.abbreviation_list_index);
}

} // namespace

// From Ryujinx
void ParseTimeZoneBinary(io::IStream* stream, TimeZoneRule& out_rule) {
    TzifHeader header = stream->Read<TzifHeader>();
    ASSERT(header.magic == make_magic4('T', 'Z', 'i', 'f'), Services,
           "Invalid TZif magic {:#x}", header.magic);

    u32 data_size = static_cast<u32>(stream->GetRemainingSize());

    u32 ttis_gmt_count = Decode(header.ttis_gmt_count);
    u32 ttis_std_count = Decode(header.ttis_std_count);
    u32 leap_count = Decode(header.leap_count);
    u32 time_count = Decode(header.time_count);
    u32 type_count = Decode(header.type_count);
    u32 char_count = Decode(header.char_count);

    ASSERT(leap_count < TimeZoneRule::MAX_LEAP_COUNT &&
               type_count < TimeZoneRule::MAX_TYPE_COUNT &&
               time_count < TimeZoneRule::MAX_TIME_COUNT &&
               char_count < TimeZoneRule::MAX_CHAR_COUNT &&
               (ttis_std_count == type_count || ttis_std_count == 0) &&
               (ttis_gmt_count == type_count || ttis_gmt_count == 0),
           Services, "Invalid header parameters");
    ASSERT((time_count * sizeof(u64) + time_count + type_count * 6 +
            char_count + leap_count * (sizeof(u64) + 4) + ttis_std_count +
            ttis_gmt_count) <= data_size,
           Services, "Insufficient data size");

    out_rule.time_count = time_count;
    out_rule.type_count = type_count;
    out_rule.char_count = char_count;

    time_count = 0;

    for (u32 i = 0; i < out_rule.time_count; i++) {
        const auto at = Decode(stream->Read<i64>());
        out_rule.type_indices[i] = 1;

        if (time_count != 0 && at <= out_rule.ats[time_count - 1]) {
            ASSERT(at >= out_rule.ats[time_count - 1], Services,
                   "Invalid at ({} < {})", at, out_rule.ats[time_count - 1]);

            out_rule.type_indices[i - 1] = 0;
            time_count--;
        }

        out_rule.ats[time_count++] = at;
    }

    time_count = 0;
    for (u32 i = 0; i < out_rule.time_count; i++) {
        const auto type_index = stream->Read<u8>();
        ASSERT(type_index < out_rule.type_count, Services,
               "Invalid type index ({} >= {})", type_index,
               out_rule.type_count);

        if (out_rule.type_indices[i] != 0)
            out_rule.type_indices[time_count++] = type_index;
    }

    out_rule.time_count = time_count;

    for (u32 i = 0; i < out_rule.type_count; i++) {
        TimeTypeInfo& type_info = out_rule.type_infos[i];
        type_info.gmt_offset = Decode(stream->Read<i32>());

        const auto is_day_saving_time = stream->Read<u8>();
        ASSERT(is_day_saving_time < 2, Services,
               "Invalid is day saving time boolean {}", is_day_saving_time);

        type_info.is_day_saving_time = (is_day_saving_time != 0);

        u32 abbreviation_list_index = stream->Read<u8>();
        ASSERT(abbreviation_list_index < TimeZoneRule::MAX_CHAR_COUNT, Services,
               "Invalid abbreviation list index {}", abbreviation_list_index);

        type_info.abbreviation_list_index = abbreviation_list_index;
    }

    stream->ReadToSpan(std::span<char>(out_rule.chars, out_rule.char_count));
    out_rule.chars[out_rule.char_count] = '\0';

    for (u32 i = 0; i < out_rule.type_count; i++) {
        if (ttis_std_count == 0) {
            out_rule.type_infos[i].is_standard_time_daylight = false;
        } else {
            const auto is_standard_time_daylight = stream->Read<u8>();
            ASSERT(is_standard_time_daylight < 2, Services,
                   "Invalid is standard time daylight boolean {}",
                   is_standard_time_daylight);

            out_rule.type_infos[i].is_standard_time_daylight =
                (is_standard_time_daylight != 0);
        }
    }

    for (u32 i = 0; i < out_rule.type_count; i++) {
        if (ttis_std_count == 0) {
            out_rule.type_infos[i].is_gmt = false;
        } else {
            const auto is_gmt = stream->Read<u8>();
            ASSERT(is_gmt < 2, Services, "Invalid is GMT boolean {}", is_gmt);

            out_rule.type_infos[i].is_gmt = (is_gmt != 0);
        }
    }

    u32 name_len = static_cast<u32>(stream->GetRemainingSize());
    ASSERT(name_len <= (TimeZoneRule::MAX_NAME_LEN + 1), Services,
           "Invalid name length {}", name_len);

    char tmp_name[TimeZoneRule::MAX_NAME_LEN + 1];
    stream->ReadToSpan(std::span<char>(tmp_name, name_len));

    if (name_len > 2 && tmp_name[0] == '\n' && tmp_name[name_len - 1] == '\n' &&
        out_rule.type_count + 2 <= TimeZoneRule::MAX_TYPE_COUNT) {
        tmp_name[name_len - 1] = '\0';

        char name[TimeZoneRule::MAX_NAME_LEN];
        std::memcpy(name, tmp_name + 1, name_len - 1);

        // TODO
        LOG_NOT_IMPLEMENTED(Services, "Time zone name parsing");
    }

    ASSERT(out_rule.type_count > 0, Services, "Invalid type count");

    if (out_rule.time_count > 1) {
        for (u32 i = 1; i < out_rule.time_count; i++) {
            if (TimeTypeEquals(out_rule, out_rule.type_indices[i],
                               out_rule.type_indices[0]) &&
                DifferByRepeat(out_rule.ats[i], out_rule.ats[0])) {
                out_rule.go_back = true;
                break;
            }
        }

        for (i32 i = static_cast<i32>(out_rule.time_count - 2); i >= 0; i--) {
            if (TimeTypeEquals(out_rule,
                               out_rule.type_indices[out_rule.time_count - 1],
                               out_rule.type_indices[i]) &&
                DifferByRepeat(out_rule.ats[out_rule.time_count - 1],
                               out_rule.ats[i])) {
                out_rule.go_ahead = true;
                break;
            }
        }
    }

    u32 default_type;
    for (default_type = 0; default_type < out_rule.time_count; default_type++) {
        if (out_rule.type_indices[default_type] == 0) {
            break;
        }
    }

    if (default_type < out_rule.time_count) {
        if (out_rule.time_count > 0 &&
            out_rule.type_infos[out_rule.type_indices[0]].is_day_saving_time) {
            default_type = out_rule.type_indices[0];
            while (true) {
                if (default_type > 0) {
                    default_type--;
                    if (!out_rule.type_infos[default_type].is_day_saving_time) {
                        break;
                    }
                } else {
                    default_type = 0;
                    while (
                        out_rule.type_infos[default_type].is_day_saving_time) {
                        if (++default_type >= out_rule.type_count) {
                            default_type = 0;
                            break;
                        }
                    }
                }
            }
        }

        out_rule.default_type = static_cast<u32>(default_type);
    } else {
        out_rule.default_type = 0;
    }
}

} // namespace hydra::horizon::services::timesrv::internal
