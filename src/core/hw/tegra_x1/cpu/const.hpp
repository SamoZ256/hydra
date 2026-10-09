#pragma once

namespace hydra::hw::tegra_x1::cpu {

constexpr u64 GUEST_PAGE_SIZE = 0x1000;

constexpr u32 MAX_STACK_TRACE_DEPTH = 32;

enum class HaltReason {
    Breakpoint,
    BrkInstruction,
    StepComplete,
    Fault,
};

} // namespace hydra::hw::tegra_x1::cpu

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::cpu::HaltReason, Breakpoint,
                       "breakpoint", BrkInstruction, "brk instruction",
                       StepComplete, "step complete", Fault, "fault")
