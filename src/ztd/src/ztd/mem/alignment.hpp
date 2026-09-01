#pragma once

#include "ztd/type_aliases.hpp"

namespace ztd::mem {

template <typename T>
constexpr auto alignDown(T value, T alignment) noexcept -> T {
    return value & ~(alignment - 1);
}

template <typename T>
constexpr auto alignUp(T value, T alignment) noexcept -> T {
    return alignDown(value + alignment - 1, alignment);
}

template <typename T>
constexpr auto ceilDivide(T value, T divisor) noexcept -> T {
    return (value + divisor - 1) / divisor;
}

} // namespace ztd::mem
