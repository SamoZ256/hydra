#pragma once

namespace hydra::hw::tegra_x1::cpu {

constexpr usize GUEST_PAGE_SIZE = 0x1000;

constexpr u32 MAX_STACK_TRACE_DEPTH = 32;

constexpr u32 CLOCK_RATE_HZ = 19'200'000;

} // namespace hydra::hw::tegra_x1::cpu
