#pragma once

#include "ztd/type_aliases.hpp"

namespace ztd::mem {

template <typename T>
auto alignDown(T value, T alignment) noexcept -> T {
    return value & ~(alignment - 1);
}

template <typename T>
auto alignUp(T value, T alignment) noexcept -> T {
    return alignDown(value + alignment - 1, alignment);
}

} // namespace ztd::mem
