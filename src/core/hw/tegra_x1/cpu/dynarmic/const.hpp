#pragma once

#include <dynarmic/interface/A64/a64.h>
#include <dynarmic/interface/A64/config.h>

#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

constexpr auto HALT_STATE_SERIALIZED = Dynarmic::HaltReason::UserDefined1;
constexpr auto HALT_REQUESTED = Dynarmic::HaltReason::UserDefined2;

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
