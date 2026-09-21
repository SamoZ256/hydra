#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/module.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {

class Builder {
  public:
    explicit Builder(Module& modul_) : modul{modul_} {
        setInsertFunction("main");
        setInsertBlock(0x0);
    }

    // Operations

    // Data
    void opCopy(const Value& dst, const Value& src) {
        addInstructionWithDst(Opcode::Copy, dst, {src});
    }
    Value opCopy(const Value& src) {
        return addInstruction(Opcode::Copy, src.getType(), {src});
    }
    Value opCast(const Value& src, Type dst_type) {
        return addInstruction(Opcode::Cast, dst_type, {src});
    }
    Value opBitCast(const Value& src, Type dst_type) {
        return addInstruction(Opcode::BitCast, dst_type, {src});
    }

    // Arithmetic
    Value opAbs(const Value& src) {
        return addInstruction(Opcode::Abs, src.getType(), {src});
    }
    Value opNeg(const Value& src) {
        return addInstruction(Opcode::Neg, src.getType(), {src});
    }
    Value opAdd(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for add ({} != {})", src_a.getType(),
                     src_b.getType());
        return addInstruction(Opcode::Add, src_a.getType(), {src_a, src_b});
    }
    Value opMultiply(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for multiply ({} != {})", src_a.getType(),
                     src_b.getType());
        return addInstruction(Opcode::Multiply, src_a.getType(),
                              {src_a, src_b});
    }
    Value opFma(const Value& src_a, const Value& src_b, const Value& src_c) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for fma ({} != {})", src_a.getType(),
                     src_b.getType());
        ASSERT_DEBUG(src_a.getType() == src_c.getType(), ShaderDecompiler,
                     "Type mismatch for fma ({} != {})", src_a.getType(),
                     src_c.getType());
        return addInstruction(Opcode::Fma, src_a.getType(),
                              {src_a, src_b, src_c});
    }
    Value opMin(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for min ({} != {})", src_a.getType(),
                     src_b.getType());
        return addInstruction(Opcode::Min, src_a.getType(), {src_a, src_b});
    }
    Value opMax(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for max ({} != {})", src_a.getType(),
                     src_b.getType());
        return addInstruction(Opcode::Max, src_a.getType(), {src_a, src_b});
    }
    Value opClamp(const Value& src_a, const Value& src_b, const Value& src_c) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for clamp ({} != {})", src_a.getType(),
                     src_b.getType());
        ASSERT_DEBUG(src_a.getType() == src_c.getType(), ShaderDecompiler,
                     "Type mismatch for clamp ({} != {})", src_a.getType(),
                     src_c.getType());
        return addInstruction(Opcode::Clamp, src_a.getType(),
                              {src_a, src_b, src_c});
    }

    // Math
    Value opIsNan(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot check for NaN with type {}", src.getType());
        return addInstruction(Opcode::IsNan, src.getType(), {src});
    }
    Value opRound(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot round with type {}", src.getType());
        return addInstruction(Opcode::Round, src.getType(), {src});
    }
    Value opFloor(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot floor with type {}", src.getType());
        return addInstruction(Opcode::Floor, src.getType(), {src});
    }
    Value opCeil(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot ceil with type {}", src.getType());
        return addInstruction(Opcode::Ceil, src.getType(), {src});
    }
    Value opTrunc(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot truncate with type {}", src.getType());
        return addInstruction(Opcode::Trunc, src.getType(), {src});
    }
    Value opReciprocal(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot reciprocal with type {}", src.getType());
        return addInstruction(Opcode::Reciprocal, src.getType(), {src});
    }
    Value opSin(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot perform sin with type {}", src.getType());
        return addInstruction(Opcode::Sin, src.getType(), {src});
    }
    Value opCos(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot perform cos with type {}", src.getType());
        return addInstruction(Opcode::Cos, src.getType(), {src});
    }
    Value opExp2(const Value& src) {
        return addInstruction(Opcode::Exp2, src.getType(), {src});
    }
    Value opLog2(const Value& src) {
        return addInstruction(Opcode::Log2, src.getType(), {src});
    }
    Value opSqrt(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot perform sqrt with type {}", src.getType());
        return addInstruction(Opcode::Sqrt, src.getType(), {src});
    }
    Value opReciprocalSqrt(const Value& src) {
        ASSERT_DEBUG(src.getType().isFloatingPoint(), ShaderDecompiler,
                     "Cannot perform reciprocal sqrt with type {}",
                     src.getType());
        return addInstruction(Opcode::ReciprocalSqrt, src.getType(), {src});
    }

    // Logical & Bitwise
    Value opNot(const Value& src) {
        ASSERT_DEBUG(src.getType() == ScalarType::Bool, ShaderDecompiler,
                     "Cannot perform not with non-boolean type {}",
                     src.getType());
        return addInstruction(Opcode::Not, ScalarType::Bool, {src});
    }
    Value opBitwiseNot(const Value& src) {
        ASSERT_DEBUG(src.getType().isScalar() &&
                         (src.getType().isInteger() ||
                          src.getType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise not with non-integer type {}",
                     src.getType());
        return addInstruction(Opcode::BitwiseNot, src.getType(), {src});
    }
    Value opBitwiseAnd(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for bitwise and ({} != {})",
                     src_a.getType(), src_b.getType());
        ASSERT_DEBUG(src_a.getType().isScalar() &&
                         (src_a.getType().isInteger() ||
                          src_a.getType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise and with non-integer type {}",
                     src_a.getType());
        return addInstruction(Opcode::BitwiseAnd, src_a.getType(),
                              {src_a, src_b});
    }
    Value opBitwiseOr(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for bitwise or ({} != {})", src_a.getType(),
                     src_b.getType());
        ASSERT_DEBUG(src_a.getType().isScalar() &&
                         (src_a.getType().isInteger() ||
                          src_a.getType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise or with non-integer type {}",
                     src_a.getType());
        return addInstruction(Opcode::BitwiseOr, src_a.getType(),
                              {src_a, src_b});
    }
    Value opBitwiseXor(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for bitwise xor ({} != {})",
                     src_a.getType(), src_b.getType());
        ASSERT_DEBUG(src_a.getType().isScalar() &&
                         (src_a.getType().isInteger() ||
                          src_a.getType() == ScalarType::Bool),
                     ShaderDecompiler,
                     "Cannot perform bitwise xor with non-integer type {}",
                     src_a.getType());
        return addInstruction(Opcode::BitwiseXor, src_a.getType(),
                              {src_a, src_b});
    }
    Value opShiftLeft(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType().isScalar() && src_a.getType().isInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_a.getType());
        ASSERT_DEBUG(src_b.getType().isScalar() && src_b.getType().isInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_b.getType());
        return addInstruction(Opcode::ShiftLeft, src_a.getType(),
                              {src_a, src_b});
    }
    Value opShiftRight(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType().isScalar() && src_a.getType().isInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_a.getType());
        ASSERT_DEBUG(src_b.getType().isScalar() && src_b.getType().isInteger(),
                     ShaderDecompiler,
                     "Cannot perform shift left with non-integer type {}",
                     src_b.getType());
        return addInstruction(Opcode::ShiftRight, src_a.getType(),
                              {src_a, src_b});
    }
    Value opBitfieldExtract(const Value& src_a, const Value& src_b,
                            const Value& src_c) {
        ASSERT_DEBUG(src_a.getType().isScalar() && src_a.getType().isInteger(),
                     ShaderDecompiler,
                     "Cannot perform bitfield extract with non-integer type {}",
                     src_a.getType());
        ASSERT_DEBUG(src_b.getType().isScalar() && src_b.getType().isInteger(),
                     ShaderDecompiler,
                     "Cannot perform bitfield extract with non-integer type {}",
                     src_b.getType());
        ASSERT_DEBUG(src_c.getType().isScalar() && src_c.getType().isInteger(),
                     ShaderDecompiler,
                     "Cannot perform bitfield extract with non-integer type {}",
                     src_c.getType());
        return addInstruction(Opcode::BitfieldExtract, src_a.getType(),
                              {src_a, src_b, src_c});
    }

    // Comparison & Selection
    Value opCompareLess(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for compare less ({} != {})",
                     src_a.getType(), src_b.getType());
        return addInstruction(Opcode::CompareLess, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value opCompareLessOrEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for compare less or equal ({} != {})",
                     src_a.getType(), src_b.getType());
        return addInstruction(Opcode::CompareLessOrEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value opCompareGreater(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for compare greater ({} != {})",
                     src_a.getType(), src_b.getType());
        return addInstruction(Opcode::CompareGreater, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value opCompareGreaterOrEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for compare greater or equal ({} != {})",
                     src_a.getType(), src_b.getType());
        return addInstruction(Opcode::CompareGreaterOrEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value opCompareEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for compare equal ({} != {})",
                     src_a.getType(), src_b.getType());
        return addInstruction(Opcode::CompareEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value opCompareNotEqual(const Value& src_a, const Value& src_b) {
        ASSERT_DEBUG(src_a.getType() == src_b.getType(), ShaderDecompiler,
                     "Type mismatch for compare not equal ({} != {})",
                     src_a.getType(), src_b.getType());
        return addInstruction(Opcode::CompareNotEqual, ScalarType::Bool,
                              {src_a, src_b});
    }
    Value opSelect(const Value& cond, const Value& src_true,
                   const Value& src_false) {
        ASSERT_DEBUG(cond.getType() == ScalarType::Bool, ShaderDecompiler,
                     "Cannot perform select with non-boolean type {}",
                     cond.getType());
        ASSERT_DEBUG(src_true.getType() == src_false.getType(),
                     ShaderDecompiler, "Type mismatch for select ({} != {})",
                     src_true.getType(), src_false.getType());
        return addInstruction(Opcode::Select, src_true.getType(),
                              {cond, src_true, src_false});
    }

    // Control flow
    void opBeginIf(const Value& cond) {
        ASSERT_DEBUG(cond.getType() == ScalarType::Bool, ShaderDecompiler,
                     "Cannot perform begin if with non-boolean type {}",
                     cond.getType());
        addInstructionWithDst(Opcode::BeginIf, std::nullopt, {cond});
    }
    void opEndIf() { addInstructionWithDst(Opcode::EndIf); }
    void opBranch(label_t target) {
        addInstructionWithDst(Opcode::Branch, std::nullopt,
                              {Value::createLabel(target)});
    }
    void opBranchConditional(const Value& cond, label_t target_true,
                             label_t target_false) {
        ASSERT_DEBUG(
            cond.getType() == ScalarType::Bool, ShaderDecompiler,
            "Cannot perform branch conditional with non-boolean type {}",
            cond.getType());
        addInstructionWithDst(Opcode::BranchConditional, std::nullopt,
                              {cond, Value::createLabel(target_true),
                               Value::createLabel(target_false)});
    }

    // Vector
    Value opVectorExtract(const Value& src, u8 index) {
        ASSERT_DEBUG(src.getType().isVector(), ShaderDecompiler,
                     "Cannot perform vector extract with non-vector type {}",
                     src.getType());
        return addInstruction(Opcode::VectorExtract,
                              src.getType().getVectorType().getElementType(),
                              {src, Value::createRawValue(index)});
    }
    void opVectorInsert(const Value& dst, const Value& src, u8 index) {
        ASSERT_DEBUG(dst.getType().isVector(), ShaderDecompiler,
                     "Cannot perform vector insert with non-vector type {}",
                     dst.getType());
        ASSERT_DEBUG(dst.getType().getVectorType().getElementType() ==
                         src.getType().getScalarType(),
                     ShaderDecompiler,
                     "Element type mismatch for vector insert ({} != {})",
                     src.getType(),
                     dst.getType().getVectorType().getElementType());
        addInstructionWithDst(Opcode::VectorInsert, dst,
                              {src, Value::createRawValue(index)});
    }
    Value opVectorConstruct(ScalarType element_type,
                            const std::vector<Value>& elements) {
        std::vector<Value> operands;
        operands.reserve(elements.size());
        for (const auto& element : elements)
            operands.push_back(element);
        return addInstruction(
            Opcode::VectorConstruct,
            Type::createVector(element_type, static_cast<u8>(elements.size())),
            operands);
    }

    // Texture
    Value opTextureSample(u32 const_buffer_index, TextureType type,
                          TextureSampleFlags flags, const Value& array_index,
                          const Value& coords, const Value& cmp_value,
                          const Value& lod) {
        ASSERT_DEBUG(coords.getType().isVector() &&
                         coords.getType().isFloatingPoint(),
                     ShaderDecompiler,
                     "Cannot perform texture sample with non-floating point "
                     "vector type {}",
                     coords.getType());
        // TODO: texture type
        return addInstruction(
            Opcode::TextureSample, Type::createVector(ScalarType::F32, 4),
            {Value::createRawValue(const_buffer_index),
             Value::createRawValue(type), Value::createRawValue(flags),
             array_index, coords, cmp_value, lod});
    }
    // TODO: more args
    Value opTextureGather(u32 const_buffer_index, const Value& coords,
                          u8 component) {
        ASSERT_DEBUG(coords.getType().isVector() &&
                         coords.getType().isFloatingPoint(),
                     ShaderDecompiler,
                     "Cannot perform texture gather with non-floating point "
                     "vector type {}",
                     coords.getType());
        // TODO: texture type
        return addInstruction(Opcode::TextureGather,
                              Type::createVector(ScalarType::F32, 4),
                              {Value::createRawValue(const_buffer_index),
                               coords, Value::createRawValue(component)});
    }
    Value opTextureQueryDimension(u32 const_buffer_index, u32 dimension) {
        return addInstruction(Opcode::TextureQueryDimension, ScalarType::U32,
                              {Value::createRawValue(const_buffer_index),
                               Value::createRawValue(dimension)});
    }

    // Exit
    void opExit() { addInstructionWithDst(Opcode::Exit); }
    void opDiscard() { addInstructionWithDst(Opcode::Discard); }

  protected:
    Module& modul;

    Function* insert_func;
    Block* insert_block;

    void addInstructionWithDst(Opcode opcode,
                               const std::optional<Value> dst = std::nullopt,
                               const std::vector<Value>& operands = {}) {
        ASSERT_DEBUG(insert_block, ShaderDecompiler, "No insert block");
        insert_block->addInstruction(opcode, dst, operands);
    }

    Value addInstruction(Opcode opcode, Type dst_type,
                         const std::vector<Value>& operands = {}) {
        ASSERT_DEBUG(insert_block, ShaderDecompiler, "No insert block");
        const auto dst = insert_block->createLocal(dst_type);
        insert_block->addInstruction(opcode, dst, operands);
        return dst;
    }

  public:
    void setInsertFunction(const std::string& name) {
        insert_func = &modul.getFunction(name);
    }

    void setInsertBlock(const label_t label) {
        ASSERT_DEBUG(insert_func, ShaderDecompiler, "No insert function");
        insert_block = &insert_func->getBlock(label);
    }
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir
