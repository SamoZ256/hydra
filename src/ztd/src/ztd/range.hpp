#pragma once

#include <algorithm>

#include "ztd/type_aliases.hpp"

namespace ztd {

template <typename T>
class Range {
  public:
    static constexpr auto fromSize(T begin_, T size) noexcept -> ztd::Range<T> {
        return ztd::Range<T>(begin_, begin_ + size);
    }

    constexpr Range() noexcept : begin{0}, end{0} {}
    constexpr Range(T begin_, T end_) noexcept : begin{begin_}, end{end_} {}

    constexpr auto operator==(const ztd::Range<T>& other) const noexcept {
        return begin == other.begin && end == other.end;
    }

    constexpr auto operator+=(T offset) noexcept {
        begin += offset;
        end += offset;
    }

    constexpr auto operator-=(T offset) noexcept {
        begin -= offset;
        end -= offset;
    }

    constexpr auto getBegin() const noexcept -> T { return begin; }
    constexpr auto setBegin(T begin_) noexcept { begin = begin_; }

    constexpr auto getEnd() const noexcept -> T { return end; }
    constexpr auto setEnd(T end_) noexcept { end = end_; }

    constexpr auto getSize() const noexcept -> T { return end - begin; }
    constexpr auto setSize(T size) noexcept { end = begin + size; }

    // Intersection
    constexpr auto contains(T value) const noexcept -> bool {
        return value >= begin && value < end;
    }
    constexpr auto contains(const ztd::Range<T>& other) const noexcept -> bool {
        return other.begin >= begin && other.end <= end;
    }

    constexpr auto intersects(const ztd::Range<T>& other) const noexcept
        -> bool {
        return begin < other.end && end > other.begin;
    }

    // Combining
    constexpr auto clampedTo(const ztd::Range<T>& bounds) const noexcept
        -> ztd::Range<T> {
        return ztd::Range<T>(std::max(begin, bounds.begin),
                             std::min(end, bounds.end));
    }

    constexpr auto merged(const ztd::Range<T>& other) const noexcept
        -> ztd::Range<T> {
        return ztd::Range<T>(std::min(begin, other.begin),
                             std::max(end, other.end));
    }

  private:
    T begin;
    T end;
};

} // namespace ztd
