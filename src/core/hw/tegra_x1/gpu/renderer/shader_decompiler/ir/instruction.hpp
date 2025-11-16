#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/value.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

enum class Opcode {
    // Basic
    Copy,
    Neg,
    Not,
    Add,
    Multiply,
    Fma,
    ShiftLeft,
    ShiftRight,
    Cast,
    Compare,
    Bitwise,
    Select,

    // Control flow
    Branch,
    BranchConditional,
    BeginIf,
    EndIf,

    // Math
    Round,
    Floor,
    Ceil,
    Trunc,
    Min,
    Max,
    Clamp,
    MathFunction,

    // Vector
    VectorExtract,
    VectorInsert,
    VectorConstruct,

    // Special
    Exit,
    Discard,
    TextureSample,
    TextureRead,
    TextureGather,
    TextureQueryDimension,
};

constexpr usize MAX_INSTRUCTION_OPERANDS = 8;

struct Instruction {
  public:
    Instruction(Opcode opcode_, const std::optional<Value> dst_,
                const std::vector<Value>& operands_)
        : opcode{opcode_}, dst{dst_}, operands{operands_} {}

    bool HasDst() const { return dst.has_value(); }
    const Value& GetDst() const { return dst.value(); }

    usize GetOperandCount() const { return operands.size(); }
    const Value& GetOperand(usize index) const {
        ASSERT_DEBUG(index < GetOperandCount(), ShaderDecompiler,
                     "Invalid operand index ({} > {})", index,
                     GetOperandCount());
        return operands[index];
    }

  private:
    Opcode opcode;
    std::optional<Value> dst;
    std::vector<Value> operands;

  public:
    GETTER(opcode, GetOpcode);
    CONST_REF_GETTER(operands, GetOperands);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Opcode, Copy, "copy",
    Neg, "neg", Not, "not", Add, "add", Multiply, "mul", Fma, "fma", ShiftLeft,
    "shl", ShiftRight, "shr", Cast, "cast", Compare, "cmp", Bitwise, "bitwise",
    Select, "select", Branch, "branch", BranchConditional, "branch_cond",
    BeginIf, "begin_if", EndIf, "end_if", Min, "min", Max, "max", MathFunction,
    "math_func", VectorExtract, "extract", VectorInsert, "insert",
    VectorConstruct, "construct", Exit, "exit", Discard, "discard",
    TextureSample, "sample", TextureRead, "read")

template <>
struct fmt::formatter<
    hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::Instruction>
    : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir::
                    Instruction& inst,
                FormatContext& ctx) const {
        std::string str = fmt::format("{:{}}", "", 1 * 2); // TODO: indent
        if (inst.HasDst())
            str += fmt::format("{} = ", inst.GetDst());
        str += fmt::format("{}", inst.GetOpcode());
        for (const auto& operand : inst.GetOperands())
            str += fmt::format(" {}", operand);

        return formatter<string_view>::format(str, ctx);
    }
};
