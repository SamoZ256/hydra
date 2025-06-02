#pragma once

#include "core/hw/tegra_x1/gpu/const.hpp"

namespace hydra::hw::tegra_x1::gpu::macro {

enum class Operation : int32_t {
    Alu = 0,
    AddImmediate = 1,
    ExtractInsert = 2,
    ExtractShiftLeftImmediate = 3,
    ExtractShiftLeftRegister = 4,
    Read = 5,
    Branch = 7,
};

enum class AluOperation : int32_t {
    Add = 0,
    AddWithCarry = 1,
    Subtract = 2,
    SubtractWithBorrow = 3,
    Xor = 8,
    Or = 9,
    And = 10,
    AndNot = 11,
    Nand = 12
};

enum class BranchCondition : int32_t {
    Zero = 0,
    NotZero = 1,
    ZeroAnnul = 2,
    NotZeroAnnul = 3,
};

enum class ResultOperation : int32_t {
    IgnoreAndFetch = 0,
    Move = 1,
    MoveAndSetMethod = 2,
    FetchAndSend = 3,
    MoveAndSend = 4,
    FetchAndSetMethod = 5,
    MoveAndSetMethodFetchAndSend = 6,
    MoveAndSetMethodSend = 7
};

constexpr usize REG_COUNT = 8;
constexpr u32 EXIT_BIT = BIT(7);

} // namespace hydra::hw::tegra_x1::gpu::macro

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::macro::AluOperation, Add,
                       "add", AddWithCarry, "add with carry", Subtract,
                       "subtract", SubtractWithBorrow, "subtract with borrow",
                       Xor, "xor", Or, "or", And, "and", AndNot, "and not",
                       Nand, "nand")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::macro::BranchCondition, Zero,
                       "zero", NotZero, "not zero", ZeroAnnul, "zero annul",
                       NotZeroAnnul, "not zero annul")

ENABLE_ENUM_FORMATTING(hydra::hw::tegra_x1::gpu::macro::ResultOperation,
                       IgnoreAndFetch, "ignore and fetch", Move, "move",
                       MoveAndSetMethod, "move and set method", FetchAndSend,
                       "fetch and send", MoveAndSend, "move and send",
                       FetchAndSetMethod, "fetch and set method",
                       MoveAndSetMethodFetchAndSend,
                       "move and set method fetch and send",
                       MoveAndSetMethodSend, "move and set method send")
