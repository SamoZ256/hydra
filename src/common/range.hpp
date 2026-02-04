#pragma once

#include "common/functions.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

template <typename T>
class Range {
  public:
    static constexpr Range<T> FromSize(T begin_, T size) {
        return Range<T>(begin_, begin_ + size);
    }

    constexpr Range() : begin{0}, end{0} {}
    constexpr Range(T begin_, T end_) : begin{begin_}, end{end_} {}

    bool operator==(const Range<T>& other) const {
        return begin == other.begin && end == other.end;
    }

    void operator+=(T offset) {
        begin += offset;
        end += offset;
    }

    void operator-=(T offset) {
        begin -= offset;
        end -= offset;
    }

    // Size
    constexpr T GetSize() const { return end - begin; }
    constexpr void SetSize(T size) { end = begin + size; }

    // Intersection
    bool Contains(T value) const { return value >= begin && value < end; }
    bool Contains(const Range<T>& other) const {
        return other.begin >= begin && other.end <= end;
    }

    bool Intersects(const Range<T>& other) const {
        return begin < other.end && end > other.begin;
    }

    // Combining
    Range<T> ClampedTo(const Range<T>& bounds) const {
        return Range<T>(std::max(begin, bounds.begin),
                        std::min(end, bounds.end));
    }

    Range<T> Union(const Range<T>& other) const {
        return Range<T>(std::min(begin, other.begin), std::max(end, other.end));
    }

  private:
    T begin;
    T end;

  public:
    CONSTEXPR_GETTER_AND_SETTER(begin, GetBegin, SetBegin);
    CONSTEXPR_GETTER_AND_SETTER(end, GetEnd, SetEnd);
};

} // namespace hydra

template <typename T>
struct fmt::formatter<hydra::Range<T>> : formatter<string_view> {
    fmt::formatter<T> value_formatter;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const hydra::Range<T>& range, FormatContext& ctx) const {
        auto out = ctx.out();

        *out++ = '<';
        out = value_formatter.format(range.GetBegin(), ctx);
        out = fmt::format_to(out, "...");
        out = value_formatter.format(range.GetEnd(), ctx);
        *out++ = ')';

        return out;
    }
};
