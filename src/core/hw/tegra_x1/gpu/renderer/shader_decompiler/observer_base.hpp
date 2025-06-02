#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

class ValueBase {};

class ObserverBase {
  public:
    virtual void SetPredCond(const PredCond pred_cond) {}
    virtual void ClearPredCond() {}
    virtual void BlockChanged() {}

    // Operations

    // Value
    virtual ValueBase* OpImmediateL(u32 imm, DataType data_type = DataType::U32,
                                    bool neg = false) {
        return nullptr;
    }
    virtual ValueBase* OpRegister(bool load, reg_t reg,
                                  DataType data_type = DataType::U32,
                                  bool neg = false) {
        return nullptr;
    }
    virtual ValueBase* OpPredicate(bool load, pred_t pred, bool not_ = false) {
        return nullptr;
    }
    virtual ValueBase* OpAttributeMemory(bool load, const AMem& amem,
                                         DataType data_type = DataType::U32,
                                         bool neg = false) {
        return nullptr;
    }
    virtual ValueBase* OpConstMemoryL(const CMem& cmem,
                                      DataType data_type = DataType::U32,
                                      bool neg = false) {
        return nullptr;
    }

    // Basic
    virtual void OpMove(ValueBase* dst, ValueBase* src) {}
    virtual ValueBase* OpAdd(ValueBase* srcA, ValueBase* srcB) {
        return nullptr;
    }
    virtual ValueBase* OpMultiply(ValueBase* srcA, ValueBase* srcB) {
        return nullptr;
    }
    virtual ValueBase* OpFloatFma(ValueBase* srcA, ValueBase* srcB,
                                  ValueBase* srcC) {
        return nullptr;
    }
    virtual ValueBase* OpShiftLeft(ValueBase* src, u32 shift) {
        return nullptr;
    }
    virtual ValueBase* OpCast(ValueBase* src, DataType dst_type) {
        return nullptr;
    }
    virtual ValueBase* OpCompare(ComparisonOperator cmp, ValueBase* srcA,
                                 ValueBase* srcB) {
        return nullptr;
    }
    virtual ValueBase* OpBinary(BinaryOperator bin, ValueBase* srcA,
                                ValueBase* srcB) {
        return nullptr;
    }
    virtual ValueBase* OpSelect(ValueBase* cond, ValueBase* src_true,
                                ValueBase* src_false) {
        return nullptr;
    }

    // Control flow
    virtual void OpSetSync(u32 target) {}
    virtual void OpSync() {}
    virtual void OpBranch(u32 target) {}
    virtual void OpExit() {}

    // Math
    virtual ValueBase* OpMin(ValueBase* srcA, ValueBase* srcB) {
        return nullptr;
    }
    virtual ValueBase* OpMax(ValueBase* srcA, ValueBase* srcB) {
        return nullptr;
    }
    virtual ValueBase* OpMathFunction(MathFunc func, ValueBase* src) {
        return nullptr;
    }

    // Special
    // TODO: remove this
    virtual ValueBase* OpInterpolate(ValueBase* src) { return nullptr; }
    virtual void OpTextureSample(ValueBase* dstA, ValueBase* dstB,
                                 ValueBase* dstC, ValueBase* dstD,
                                 u32 const_buffer_index, ValueBase* coords_x,
                                 ValueBase* coords_y) {}
    virtual void OpDiscard() {}

    // Setters
    virtual void SetPC(const u32 pc_) { pc = pc_; }

  protected:
    u32 pc{0};
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
