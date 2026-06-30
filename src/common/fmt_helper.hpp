#pragma once

#include "ztd/ztd.hpp"

#include <cstdlib>
#include <fmt/core.h>

#define ENUM_FORMAT_CASE(type, c, name)                                        \
    case type::c:                                                              \
        res = name;                                                            \
        break;

#define ENABLE_ENUM_FORMATTING(type, ...)                                      \
    template <>                                                                \
    struct fmt::formatter<type> : formatter<string_view> {                     \
        template <typename FormatContext>                                      \
        auto format(type value, FormatContext& ctx) const {                    \
            std::string_view res;                                              \
            switch (value) {                                                   \
                ZTD_FOR_EACH_1_2(ENUM_FORMAT_CASE, type, __VA_ARGS__)          \
            default:                                                           \
                return formatter<string_view>::format(                         \
                    fmt::format("unknown ({})",                                \
                                static_cast<hydra::u64>(value)),               \
                    ctx);                                                      \
                break;                                                         \
            }                                                                  \
            return formatter<string_view>::format(res, ctx);                   \
        }                                                                      \
    };

#define STRUCT_FORMAT_CASE(member, f, name)                                    \
    fmt::format(name ": {" f "}", value.member),

#define ENABLE_STRUCT_FORMATTING(type, ...)                                    \
    template <>                                                                \
    struct fmt::formatter<type> : formatter<string_view> {                     \
        template <typename FormatContext>                                      \
        auto format(const type& value, FormatContext& ctx) const {             \
            /* TODO: make this more efficient */                               \
            std::string res = fmt::format(                                     \
                "{}", fmt::join(std::array{ZTD_FOR_EACH_0_3(                   \
                                    STRUCT_FORMAT_CASE, __VA_ARGS__)},         \
                                ", "));                                        \
            return formatter<string_view>::format(std::move(res), ctx);        \
        }                                                                      \
    };

#define ENUM_CAST_CASE(type, value, n)                                         \
    if (value_str == n)                                                        \
        return type::value;

#define ENABLE_ENUM_CASTING(namespc, type, ...)                                \
    namespace namespc {                                                        \
    inline std::optional<type> To##type(std::string_view value_str) {          \
        ZTD_FOR_EACH_1_2(ENUM_CAST_CASE, type, __VA_ARGS__)                    \
        return std::nullopt;                                                   \
    }                                                                          \
    }

#define ENABLE_ENUM_FORMATTING_AND_CASTING(namespc, type, ...)                 \
    ENABLE_ENUM_FORMATTING(namespc::type, __VA_ARGS__)                         \
    ENABLE_ENUM_CASTING(namespc, type, __VA_ARGS__)

#define ENUM_BIT_TEST(type, c, n)                                              \
    if (any(value & type::c)) {                                                \
        if (added)                                                             \
            name += " | ";                                                     \
        else                                                                   \
            added = true;                                                      \
        name += n;                                                             \
    }

#define ENABLE_ENUM_FLAGS_FORMATTING(type, ...)                                \
    template <>                                                                \
    struct fmt::formatter<type> : formatter<string_view> {                     \
        template <typename FormatContext>                                      \
        auto format(type value, FormatContext& ctx) const {                    \
            std::string name;                                                  \
            bool added = false;                                                \
            ZTD_FOR_EACH_1_2(ENUM_BIT_TEST, type, __VA_ARGS__)                 \
            if (!added)                                                        \
                name = "none";                                                 \
            return formatter<string_view>::format(name, ctx);                  \
        }                                                                      \
    };

template <typename T>
struct fmt::formatter<ztd::Range<T>> : formatter<string_view> {
    fmt::formatter<T> value_formatter;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const ztd::Range<T>& range, FormatContext& ctx) const {
        auto out = ctx.out();

        *out++ = '<';
        out = value_formatter.format(range.getBegin(), ctx);
        out = fmt::format_to(out, ", ");
        out = value_formatter.format(range.getEnd(), ctx);
        *out++ = ')';

        return out;
    }
};
