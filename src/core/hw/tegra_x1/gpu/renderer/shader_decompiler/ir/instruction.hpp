#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/value.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

enum class Opcode {
    // Data
    Copy,
    Cast,

    // Arithmetic
    Abs,
    Neg,
    Add,
    Multiply,
    Fma,
    Min,
    Max,
    Clamp,

    // Math
    IsNan,
    Round,
    Floor,
    Ceil,
    Trunc,
    Reciprocal,
    Sin,
    Cos,
    Exp2,
    Log2,
    Sqrt,
    ReciprocalSqrt,

    // Logical & Bitwise
    Not,
    BitwiseNot,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    ShiftLeft,
    ShiftRight,

    // Comparison & Selection
    CompareLess,
    CompareLessOrEqual,
    CompareGreater,
    CompareGreaterOrEqual,
    CompareEqual,
    CompareNotEqual,
    Select,

    // Control Flow
    BeginIf,
    EndIf,
    Branch,
    BranchConditional,

    // Vector
    VectorExtract,
    VectorInsert,
    VectorConstruct,

    // Texture
    TextureSample,
    TextureRead,
    TextureGather,
    TextureQueryDimension,

    // Exit
    Exit,
    Discard,
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
    Cast, "cast", Abs, "abs", Neg, "neg", Add, "add", Multiply, "mul", Fma,
    "fma", Min, "min", Max, "max", Clamp, "clamp", IsNan, "is_nan", Round,
    "round", Floor, "floor", Ceil, "ceil", Trunc, "trunc", Reciprocal,
    "reciprocal", Sin, "sin", Cos, "cos", Exp2, "exp2", Log2, "log2", Sqrt,
    "sqrt", ReciprocalSqrt, "rsqrt", Not, "not", BitwiseNot, "bitwise_not",
    BitwiseAnd, "bitwise_and", BitwiseOr, "bitwise_or", BitwiseXor,
    "bitwise_xor", ShiftLeft, "shl", ShiftRight, "shr", CompareLess, "cmp_lt",
    CompareLessOrEqual, "cmp_le", CompareGreater, "cmp_gt",
    CompareGreaterOrEqual, "cmp_ge", CompareEqual, "cmp_eq", CompareNotEqual,
    "cmp_ne", Select, "select", BeginIf, "begin_if", EndIf, "end_if", Branch,
    "branch", BranchConditional, "branch_cond", VectorExtract, "vec_extract",
    VectorInsert, "vec_insert", VectorConstruct, "vec_construct", TextureSample,
    "tex_sample", TextureRead, "tex_read", TextureGather, "tex_gather",
    TextureQueryDimension, "tex_query_dim", Exit, "exit", Discard, "discard")

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
