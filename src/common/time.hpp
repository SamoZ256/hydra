#pragma once

// TODO: support cross-platform time functions

#include <chrono>
#include <mach/mach_time.h>

#include "common/types.hpp"

using namespace std::chrono_literals;

namespace hydra {

inline u64 get_absolute_time() { return mach_absolute_time(); }

} // namespace hydra
