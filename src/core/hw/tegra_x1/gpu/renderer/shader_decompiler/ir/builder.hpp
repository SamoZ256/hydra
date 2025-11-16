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

    // Basic
    void OpCopy(const Value& dst, const Value& src) {
        AddInstructionWithDst(Opcode::Copy, dst, {src});
    }
    Value OpNeg(const Value& src) { return AddInstruction(Opcode::Neg, {src}); }
    Value OpNot(const Value& src) { return AddInstruction(Opcode::Not, {src}); }
    Value OpAdd(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Add, {srcA, srcB});
    }
    Value OpMultiply(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Multiply, {srcA, srcB});
    }
    Value OpFma(const Value& srcA, const Value& srcB, const Value& srcC) {
        return AddInstruction(Opcode::Fma, {srcA, srcB, srcC});
    }
    Value OpShiftLeft(const Value& src, u32 shift) {
        return AddInstruction(Opcode::ShiftLeft, {src, Value::RawValue(shift)});
    }
    Value OpShiftRight(const Value& src, u32 shift) {
        return AddInstruction(Opcode::ShiftRight,
                              {src, Value::RawValue(shift)});
    }
    Value OpCast(const Value& src, DataType dst_type) {
        return AddInstruction(Opcode::Cast, {src, Value::RawValue(dst_type)});
    }
    Value OpCompare(ComparisonOp op, const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Compare,
                              {Value::RawValue(op), srcA, srcB});
    }
    Value OpBitwise(BitwiseOp op, const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Bitwise,
                              {Value::RawValue(op), srcA, srcB});
    }
    Value OpSelect(const Value& cond, const Value& src_true,
                   const Value& src_false) {
        return AddInstruction(Opcode::Select, {cond, src_true, src_false});
    }

    // Control flow
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
    void OpBeginIf(const Value& cond) {
        AddInstructionWithDst(Opcode::BeginIf, std::nullopt, {cond});
    }
    void OpEndIf() { AddInstructionWithDst(Opcode::EndIf); }

    // Math
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
    Value OpMin(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Min, {srcA, srcB});
    }
    Value OpMax(const Value& srcA, const Value& srcB) {
        return AddInstruction(Opcode::Max, {srcA, srcB});
    }
    Value OpClamp(const Value& srcA, const Value& srcB, const Value& srcC) {
        return AddInstruction(Opcode::Clamp, {srcA, srcB, srcC});
    }
    Value OpMathFunction(MathFunc func, const Value& src) {
        return AddInstruction(Opcode::MathFunction,
                              {Value::RawValue(func), src});
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

    // Special
    void OpExit() { AddInstructionWithDst(Opcode::Exit); }
    void OpDiscard() { AddInstructionWithDst(Opcode::Discard); }
    Value OpTextureSample(u32 const_buffer_index, const Value& coords) {
        return AddInstruction(Opcode::TextureSample,
                              {Value::RawValue(const_buffer_index), coords});
    }
    Value OpTextureRead(u32 const_buffer_index, const Value& coords) {
        return AddInstruction(Opcode::TextureRead,
                              {Value::RawValue(const_buffer_index), coords});
    }
    Value OpTextureGather(u32 const_buffer_index, const Value& coords,
                          TextureComponent component) {
        return AddInstruction(Opcode::TextureGather,
                              {Value::RawValue(const_buffer_index), coords,
                               Value::RawValue(component)});
    }
    Value OpTextureQueryDimension(u32 const_buffer_index, u32 dimension) {
        return AddInstruction(
            Opcode::TextureQueryDimension,
            {Value::RawValue(const_buffer_index), Value::RawValue(dimension)});
    }

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
