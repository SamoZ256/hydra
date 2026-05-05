#pragma once

#include <chrono>

#include "common/types.hpp"

using namespace std::chrono_literals;

namespace hydra {

inline u64 get_absolute_time() {
    auto dur = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
    return static_cast<u64>(dur.count());
}

} // namespace hydra
