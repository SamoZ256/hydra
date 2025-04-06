#pragma once

#include "hw/tegra_x1/gpu/renderer/shader_decompiler/const.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler {

class ObserverBase {
  public:
    // Operations
    virtual void OpExit() = 0;
    virtual void OpMove(reg_t dst, Operand src) = 0;
    virtual void OpFloatAdd(reg_t dst, reg_t src1, Operand src2) = 0;
    virtual void OpFloatMultiply(reg_t dst, reg_t src1, Operand src2) = 0;
    virtual void OpFloatFma(reg_t dst, reg_t src1, Operand src2,
                            reg_t src3) = 0;
    virtual void OpShiftLeft(reg_t dst, reg_t src, u32 shift) = 0;
    virtual void OpMathFunction(MathFunc func, reg_t dst, reg_t src) = 0;
    virtual void OpLoad(reg_t dst, Operand src) = 0;
    virtual void OpStore(AMem dst, reg_t src) = 0;
    virtual void OpInterpolate(reg_t dst, AMem src) = 0;
    virtual void OpTextureSample(reg_t dst, u32 index, reg_t coords) = 0;

  private:
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer::ShaderDecompiler
