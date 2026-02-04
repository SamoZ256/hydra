#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/module.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

class Builder {
  public:
    Builder(Module& modul_) : modul{modul_} {
        SetInsertFunction("main");
        SetInsertBlock(0x0);
    }

    // Operations

    // Data
    void OpCopy(const Value& dst, const Value& src) {
        AddInstructionWithDst(Opcode::Copy, dst, {src});
    }
    Value OpCopy(const Value& src) {
        return AddInstruction(Opcode::Copy, src.GetType(), {src});
    }
    Value OpCast(const Value& src, Type dst_type) {
        return AddInstruction(Opcode::Cast, dst_type, {src});
    }
    Value OpBitCast(const Value& src, Type dst_type) {
        return AddInstruction(Opcode::BitCast, dst_type, {src});
    }

    // Arithmetic
    Value OpAbs(const Value& src) {
        return AddInstruction(Opcode::Abs, src.GetType(), {src});
    }
    Value OpNeg(const Value& src) {
        return AddInstruction(Opcode::Neg, src.GetType(), {src});
    }
    Value OpAdd(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for add ({} != {})", src_a.GetType(),
                     src_b.GetType());
        return AddInstruction(Opcode::Add, src_a.GetType(), {src_a, src_b});
    }
    Value OpMultiply(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for multiply ({} != {})", src_a.GetType(),
                     src_b.GetType());
        return AddInstruction(Opcode::Multiply, src_a.GetType(),
                              {src_a, src_b});
    }
    Value OpFma(const Value& src_a, const Value& src_b, const Value& src_c) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for fma ({} != {})", src_a.GetType(),
                     src_b.GetType());
        ASSERT_DEBUG(src_a.GetType() == src_c.GetType(), ShaderDecompiler,
                     "Type mismatch for fma ({} != {})", src_a.GetType(),
                     src_c.GetType());
        return AddInstruction(Opcode::Fma, src_a.GetType(),
                              {src_a, src_b, src_c});
    }
    Value OpMin(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for min ({} != {})", src_a.GetType(),
                     src_b.GetType());
        return AddInstruction(Opcode::Min, src_a.GetType(), {src_a, src_b});
    }
    Value OpMax(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for max ({} != {})", src_a.GetType(),
                     src_b.GetType());
        return AddInstruction(Opcode::Max, src_a.GetType(), {src_a, src_b});
    }
    Value OpClamp(const Value& src_a, const Value& src_b, const Value& src_c) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for clamp ({} != {})", src_a.GetType(),
                     src_b.GetType());
        ASSERT_DEBUG(src_a.GetType() == src_c.GetType(), ShaderDecompiler,
                     "Type mismatch for clamp ({} != {})", src_a.GetType(),
                     src_c.GetType());
        return AddInstruction(Opcode::Clamp, src_a.GetType(),
                              {src_a, src_b, src_c});
    }

    // Math
    Value OpIsNan(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot check for NaN with type {}", src.GetType());
        return AddInstruction(Opcode::IsNan, src.GetType(), {src});
    }
    Value OpRound(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot round with type {}", src.GetType());
        return AddInstruction(Opcode::Round, src.GetType(), {src});
    }
    Value OpFloor(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot floor with type {}", src.GetType());
        return AddInstruction(Opcode::Floor, src.GetType(), {src});
    }
    Value OpCeil(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot ceil with type {}", src.GetType());
        return AddInstruction(Opcode::Ceil, src.GetType(), {src});
    }
    Value OpTrunc(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot truncate with type {}", src.GetType());
        return AddInstruction(Opcode::Trunc, src.GetType(), {src});
    }
    Value OpReciprocal(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot reciprocal with type {}", src.GetType());
        return AddInstruction(Opcode::Reciprocal, src.GetType(), {src});
    }
    Value OpSin(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot perform sin with type {}", src.GetType());
        return AddInstruction(Opcode::Sin, src.GetType(), {src});
    }
    Value OpCos(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot perform cos with type {}", src.GetType());
        return AddInstruction(Opcode::Cos, src.GetType(), {src});
    }
    Value OpExp2(const Value& src) {
        return AddInstruction(Opcode::Exp2, src.GetType(), {src});
    }
    Value OpLog2(const Value& src) {
        return AddInstruction(Opcode::Log2, src.GetType(), {src});
    }
    Value OpSqrt(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot perform sqrt with type {}", src.GetType());
        return AddInstruction(Opcode::Sqrt, src.GetType(), {src});
    }
    Value OpReciprocalSqrt(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsFloatingPoint(), ShaderDecompiler,
                     "Cannot perform reciprocal sqrt with type {}",
                     src.GetType());
        return AddInstruction(Opcode::ReciprocalSqrt, src.GetType(), {src});
    }

    // Logical & Bitwise
    Value OpNot(const Value& src) {
        ASSERT_DEBUG(src.GetType() == ScalarType::Bool, ShaderDecompiler,
                     "Cannot perform not with non-boolean type {}",
                     src.GetType());
        return AddInstruction(Opcode::Not, ScalarType::Bool, {src});
    }
    Value OpBitwiseNot(const Value& src) {
        ASSERT_DEBUG(src.GetType().IsScalar() &&
                         (src.GetType().IsInteger() ||
                          src.GetType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise not with non-integer type {}",
                     src.GetType());
        return AddInstruction(Opcode::BitwiseNot, src.GetType(), {src});
    }
    Value OpBitwiseAnd(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for bitwise and ({} != {})",
                     src_a.GetType(), src_b.GetType());
        ASSERT_DEBUG(src_a.GetType().IsScalar() &&
                         (src_a.GetType().IsInteger() ||
                          src_a.GetType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise and with non-integer type {}",
                     src_a.GetType());
        return AddInstruction(Opcode::BitwiseAnd, src_a.GetType(),
                              {src_a, src_b});
    }
    Value OpBitwiseOr(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for bitwise or ({} != {})", src_a.GetType(),
                     src_b.GetType());
        ASSERT_DEBUG(src_a.GetType().IsScalar() &&
                         (src_a.GetType().IsInteger() ||
                          src_a.GetType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise or with non-integer type {}",
                     src_a.GetType());
        return AddInstruction(Opcode::BitwiseOr, src_a.GetType(),
                              {src_a, src_b});
    }
    Value OpBitwiseXor(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for bitwise xor ({} != {})",
                     src_a.GetType(), src_b.GetType());
        ASSERT_DEBUG(src_a.GetType().IsScalar() &&
                         (src_a.GetType().IsInteger() ||
                          src_a.GetType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise xor with non-integer type {}",
                     src_a.GetType());
        return AddInstruction(Opcode::BitwiseXor, src_a.GetType(),
                              {src_a, src_b});
    }
    Value OpShiftLeft(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType().IsScalar() && src_a.GetType().IsInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_a.GetType());
        ASSERT_DEBUG(src_b.GetType().IsScalar() && src_b.GetType().IsInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_b.GetType());
        return AddInstruction(Opcode::ShiftLeft, src_a.GetType(),
                              {src_a, src_b});
    }
    Value OpShiftRight(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType().IsScalar() && src_a.GetType().IsInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_a.GetType());
        ASSERT_DEBUG(src_b.GetType().IsScalar() && src_b.GetType().IsInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_b.GetType());
        return AddInstruction(Opcode::ShiftRight, src_a.GetType(),
                              {src_a, src_b});
    }
    Value OpBitfieldExtract(const Value& src_a, const Value& src_b,
                            const Value& src_c) {
        ASSERT_DEBUG(src_a.GetType().IsScalar() && src_a.GetType().IsInteger(),
                     ShaderDecompiler,
                     "Cannot perform bitfield extract with non-integer type {}",
                     src_a.GetType());
        ASSERT_DEBUG(src_b.GetType().IsScalar() && src_b.GetType().IsInteger(),
                     ShaderDecompiler,
                     "Cannot perform bitfield extract with non-integer type {}",
                     src_b.GetType());
        ASSERT_DEBUG(src_c.GetType().IsScalar() && src_c.GetType().IsInteger(),
                     ShaderDecompiler,
                     "Cannot perform bitfield extract with non-integer type {}",
                     src_c.GetType());
        return AddInstruction(Opcode::BitfieldExtract, src_a.GetType(),
                              {src_a, src_b, src_c});
    }

    // Comparison & Selection
    Value OpCompareLess(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for compare less ({} != {})",
                     src_a.GetType(), src_b.GetType());
        return AddInstruction(Opcode::CompareLess, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value OpCompareLessOrEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for compare less or equal ({} != {})",
                     src_a.GetType(), src_b.GetType());
        return AddInstruction(Opcode::CompareLessOrEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value OpCompareGreater(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for compare greater ({} != {})",
                     src_a.GetType(), src_b.GetType());
        return AddInstruction(Opcode::CompareGreater, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value OpCompareGreaterOrEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for compare greater or equal ({} != {})",
                     src_a.GetType(), src_b.GetType());
        return AddInstruction(Opcode::CompareGreaterOrEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value OpCompareEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for compare equal ({} != {})",
                     src_a.GetType(), src_b.GetType());
        return AddInstruction(Opcode::CompareEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value OpCompareNotEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.GetType() == src_b.GetType(), ShaderDecompiler,
                     "Type mismatch for compare not equal ({} != {})",
                     src_a.GetType(), src_b.GetType());
        return AddInstruction(Opcode::CompareNotEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value OpSelect(const Value& cond, const Value& src_true,
                   const Value& src_false) {
        ASSERT_DEBUG(cond.GetType() == ScalarType::Bool, ShaderDecompiler,
                     "Cannot perform select with non-boolean type {}",
                     cond.GetType());
        ASSERT_DEBUG(src_true.GetType() == src_false.GetType(),
                     ShaderDecompiler, "Type mismatch for select ({} != {})",
                     src_true.GetType(), src_false.GetType());
        return AddInstruction(Opcode::Select, src_true.GetType(),
                              {cond, src_true, src_false});
    }

    // Control flow
    void OpBeginIf(const Value& cond) {
        ASSERT_DEBUG(cond.GetType() == ScalarType::Bool, ShaderDecompiler,
                     "Cannot perform begin if with non-boolean type {}",
                     cond.GetType());
        AddInstructionWithDst(Opcode::BeginIf, std::nullopt, {cond});
    }
    void OpEndIf() { AddInstructionWithDst(Opcode::EndIf); }
    void OpBranch(label_t target) {
        AddInstructionWithDst(Opcode::Branch, std::nullopt,
                              {Value::Label(target)});
    }
    void OpBranchConditional(const Value& cond, label_t target_true,
                             label_t target_false) {
        ASSERT_DEBUG(
            cond.GetType() == ScalarType::Bool, ShaderDecompiler,
            "Cannot perform branch conditional with non-boolean type {}",
            cond.GetType());
        AddInstructionWithDst(
            Opcode::BranchConditional, std::nullopt,
            {cond, Value::Label(target_true), Value::Label(target_false)});
    }

    // Vector
    Value OpVectorExtract(const Value& src, u8 index) {
        ASSERT_DEBUG(src.GetType().IsVector(), ShaderDecompiler,
                     "Cannot perform vector extract with non-vector type {}",
                     src.GetType());
        return AddInstruction(Opcode::VectorExtract,
                              src.GetType().GetVectorType().GetElementType(),
                              {src, Value::RawValue(index)});
    }
    void OpVectorInsert(const Value& dst, const Value& src, u8 index) {
        ASSERT_DEBUG(dst.GetType().IsVector(), ShaderDecompiler,
                     "Cannot perform vector insert with non-vector type {}",
                     dst.GetType());
        ASSERT_DEBUG(dst.GetType().GetVectorType().GetElementType() ==
                         src.GetType().GetScalarType(),
                     ShaderDecompiler,
                     "Element type mismatch for vector insert ({} != {})",
                     src.GetType(),
                     dst.GetType().GetVectorType().GetElementType());
        AddInstructionWithDst(Opcode::VectorInsert, dst,
                              {src, Value::RawValue(index)});
    }
    Value OpVectorConstruct(ScalarType element_type,
                            const std::vector<Value>& elements) {
        std::vector<Value> operands;
        for (const auto& element : elements)
            operands.push_back(element);
        return AddInstruction(
            Opcode::VectorConstruct,
            Type::Vector(element_type, static_cast<u8>(elements.size())),
            operands);
    }

    // Texture
    Value OpTextureSample(u32 const_buffer_index, TextureType type,
                          TextureSampleFlags flags, const Value& array_index,
                          const Value& coords, const Value& cmp_value,
                          const Value& lod) {
        ASSERT_DEBUG(coords.GetType().IsVector() &&
                         coords.GetType().IsFloatingPoint(),
                     ShaderDecompiler,
                     "Cannot perform texture sample with non-floating point "
                     "vector type {}",
                     coords.GetType());
        // TODO: texture type
        return AddInstruction(
            Opcode::TextureSample, Type::Vector(ScalarType::F32, 4),
            {Value::RawValue(const_buffer_index), Value::RawValue(type),
             Value::RawValue(flags), array_index, coords, cmp_value, lod});
    }
    // TODO: more args
    Value OpTextureGather(u32 const_buffer_index, const Value& coords,
                          u8 component) {
        ASSERT_DEBUG(coords.GetType().IsVector() &&
                         coords.GetType().IsFloatingPoint(),
                     ShaderDecompiler,
                     "Cannot perform texture gather with non-floating point "
                     "vector type {}",
                     coords.GetType());
        // TODO: texture type
        return AddInstruction(Opcode::TextureGather,
                              Type::Vector(ScalarType::F32, 4),
                              {Value::RawValue(const_buffer_index), coords,
                               Value::RawValue(component)});
    }
    Value OpTextureQueryDimension(u32 const_buffer_index, u32 dimension) {
        return AddInstruction(
            Opcode::TextureQueryDimension, ScalarType::U32,
            {Value::RawValue(const_buffer_index), Value::RawValue(dimension)});
    }

    // Exit
    void OpExit() { AddInstructionWithDst(Opcode::Exit); }
    void OpDiscard() { AddInstructionWithDst(Opcode::Discard); }

  protected:
    Module& modul;

    Function* insert_func;
    Block* insert_block;

    void AddInstructionWithDst(Opcode opcode,
                               const std::optional<Value> dst = std::nullopt,
                               const std::vector<Value>& operands = {}) {
        ASSERT_DEBUG(insert_block, ShaderDecompiler, "No insert block");
        insert_block->AddInstruction(opcode, dst, operands);
    }

    Value AddInstruction(Opcode opcode, Type dst_type,
                         const std::vector<Value>& operands = {}) {
        ASSERT_DEBUG(insert_block, ShaderDecompiler, "No insert block");
        const auto dst = insert_block->CreateLocal(dst_type);
        insert_block->AddInstruction(opcode, dst, operands);
        return dst;
    }

  public:
    void SetInsertFunction(const std::string& name) {
        insert_func = &modul.GetFunction(name);
    }

    void SetInsertBlock(const label_t label) {
        ASSERT_DEBUG(insert_func, ShaderDecompiler, "No insert function");
        insert_block = &insert_func->GetBlock(label);
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir
