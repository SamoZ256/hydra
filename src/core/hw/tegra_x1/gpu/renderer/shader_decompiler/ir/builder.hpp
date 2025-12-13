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
    Value OpCast(const Value& src, DataType dst_type) {
        return AddInstruction(Opcode::Cast, {src, Value::RawValue(dst_type)});
    }

    // Arithmetic
    Value OpAbs(const Value& src) { return AddInstruction(Opcode::Abs, {src}); }
    Value OpNeg(const Value& src) { return AddInstruction(Opcode::Neg, {src}); }
    Value OpAdd(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Add, {srcA, srcB});
    }
    Value OpMultiply(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Multiply, {srcA, srcB});
    }
    Value OpFma(const Value& srcA, const Value& srcB, const Value& srcC) {
        return AddInstruction(Opcode::Fma, {srcA, srcB, srcC});
    }
    Value OpMin(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Min, {srcA, srcB});
    }
    Value OpMax(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Max, {srcA, srcB});
    }
    Value OpClamp(const Value& srcA, const Value& srcB, const Value& srcC) {
        return AddInstruction(Opcode::Clamp, {srcA, srcB, srcC});
    }

    // Math
    Value OpIsNan(const Value& src) {
        return AddInstruction(Opcode::IsNan, {src});
    }
    Value OpRound(const Value& src) {
        return AddInstruction(Opcode::Round, {src});
    }
    Value OpFloor(const Value& src) {
        return AddInstruction(Opcode::Floor, {src});
    }
    Value OpCeil(const Value& src) {
        return AddInstruction(Opcode::Ceil, {src});
    }
    Value OpTrunc(const Value& src) {
        return AddInstruction(Opcode::Trunc, {src});
    }
    Value OpReciprocal(const Value& src) {
        return AddInstruction(Opcode::Reciprocal, {src});
    }
    Value OpSin(const Value& src) { return AddInstruction(Opcode::Sin, {src}); }
    Value OpCos(const Value& src) { return AddInstruction(Opcode::Cos, {src}); }
    Value OpExp2(const Value& src) {
        return AddInstruction(Opcode::Exp2, {src});
    }
    Value OpLog2(const Value& src) {
        return AddInstruction(Opcode::Log2, {src});
    }
    Value OpSqrt(const Value& src) {
        return AddInstruction(Opcode::Sqrt, {src});
    }
    Value OpReciprocalSqrt(const Value& src) {
        return AddInstruction(Opcode::ReciprocalSqrt, {src});
    }

    // Logical & Bitwise
    Value OpNot(const Value& src) { return AddInstruction(Opcode::Not, {src}); }
    Value OpBitwiseNot(const Value& src) {
        return AddInstruction(Opcode::BitwiseNot, {src});
    }
    Value OpBitwiseAnd(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::BitwiseAnd, {srcA, srcB});
    }
    Value OpBitwiseOr(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::BitwiseOr, {srcA, srcB});
    }
    Value OpBitwiseXor(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::BitwiseXor, {srcA, srcB});
    }
    Value OpShiftLeft(const Value& src_a, const Value& src_b) {
        return AddInstruction(Opcode::ShiftLeft, {src_a, src_b});
    }
    Value OpShiftRight(const Value& src_a, const Value& src_b) {
        return AddInstruction(Opcode::ShiftRight, {src_a, src_b});
    }

    // Comparison & Selection
    Value OpCompareLess(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::CompareLess, {srcA, srcB});
    }
    Value OpCompareLessOrEqual(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::CompareLessOrEqual, {srcA, srcB});
    }
    Value OpCompareGreater(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::CompareGreater, {srcA, srcB});
    }
    Value OpCompareGreaterOrEqual(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::CompareGreaterOrEqual, {srcA, srcB});
    }
    Value OpCompareEqual(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::CompareEqual, {srcA, srcB});
    }
    Value OpCompareNotEqual(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::CompareNotEqual, {srcA, srcB});
    }
    Value OpSelect(const Value& cond, const Value& src_true,
                   const Value& src_false) {
        return AddInstruction(Opcode::Select, {cond, src_true, src_false});
    }

    // Control flow
    void OpBeginIf(const Value& cond) {
        AddInstructionWithDst(Opcode::BeginIf, std::nullopt, {cond});
    }
    void OpEndIf() { AddInstructionWithDst(Opcode::EndIf); }
    void OpBranch(label_t target) {
        AddInstructionWithDst(Opcode::Branch, std::nullopt,
                              {Value::Label(target)});
    }
    void OpBranchConditional(const Value& cond, label_t target_true,
                             label_t target_false) {
        AddInstructionWithDst(
            Opcode::BranchConditional, std::nullopt,
            {cond, Value::Label(target_true), Value::Label(target_false)});
    }

    // Vector
    Value OpVectorExtract(const Value& src, u32 index) {
        return AddInstruction(Opcode::VectorExtract,
                              {src, Value::RawValue(index)});
    }
    void OpVectorInsert(const Value& dst, const Value& src, u32 index) {
        AddInstructionWithDst(Opcode::VectorInsert, dst,
                              {src, Value::RawValue(index)});
    }
    Value OpVectorConstruct(DataType data_type,
                            const std::vector<Value>& elements) {
        std::vector<Value> operands = {Value::RawValue(data_type)};
        for (const auto& element : elements)
            operands.push_back(element);
        return AddInstruction(Opcode::VectorConstruct, operands);
    }

    // Texture
    Value OpTextureSample(u32 const_buffer_index, const Value& coords) {
        return AddInstruction(Opcode::TextureSample,
                              {Value::RawValue(const_buffer_index), coords});
    }
    Value OpTextureRead(u32 const_buffer_index, const Value& coords) {
        return AddInstruction(Opcode::TextureRead,
                              {Value::RawValue(const_buffer_index), coords});
    }
    Value OpTextureGather(u32 const_buffer_index, const Value& coords,
                          u8 component) {
        return AddInstruction(Opcode::TextureGather,
                              {Value::RawValue(const_buffer_index), coords,
                               Value::RawValue(component)});
    }
    Value OpTextureQueryDimension(u32 const_buffer_index, u32 dimension) {
        return AddInstruction(
            Opcode::TextureQueryDimension,
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
        insert_block->AddInstructionWithDst(opcode, dst, operands);
    }

    Value AddInstruction(Opcode opcode,
                         const std::vector<Value>& operands = {}) {
        ASSERT_DEBUG(insert_block, ShaderDecompiler, "No insert block");
        return insert_block->AddInstruction(opcode, operands);
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
