#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/value.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {
class GuestShaderState;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir {
class Instruction;
class Function;
class Module;
} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::ir

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {
class MemoryAnalyzer;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen {

class Emitter {
  public:
    Emitter(const DecompilerContext& context_,
            const analyzer::MemoryAnalyzer& memory_analyzer_,
            const GuestShaderState& state_, std::vector<u8>& out_code_,
            ResourceMapping& out_resource_mapping_)
        : context{context_}, memory_analyzer{memory_analyzer_}, state{state_},
          out_code{out_code_}, out_resource_mapping{out_resource_mapping_} {}
    virtual ~Emitter() {}

    void Emit(const ir::Module& modul);

  protected:
    const DecompilerContext& context;
    const analyzer::MemoryAnalyzer& memory_analyzer;
    const GuestShaderState& state;

    std::vector<u8>& out_code;
    ResourceMapping& out_resource_mapping;

    virtual void Start() = 0;
    virtual void Finish() = 0;

    void EmitInstruction(const ir::Instruction& inst);

    // Emit

    virtual void EmitFunction(const ir::Function& func) = 0;

    // Basic
    virtual void EmitCopy(const ir::Value& dst, const ir::Value& src) = 0;
    virtual void EmitNeg(const ir::Value& dst, const ir::Value& src) = 0;
    virtual void EmitNot(const ir::Value& dst, const ir::Value& src) = 0;
    virtual void EmitAdd(const ir::Value& dst, const ir::Value& srcA,
                         const ir::Value& srcB) = 0;
    virtual void EmitMultiply(const ir::Value& dst, const ir::Value& srcA,
                              const ir::Value& srcB) = 0;
    virtual void EmitFma(const ir::Value& dst, const ir::Value& srcA,
                         const ir::Value& srcB, const ir::Value& srcC) = 0;
    virtual void EmitShiftLeft(const ir::Value& dst, const ir::Value& src,
                               u32 shift) = 0;
    virtual void EmitShiftRight(const ir::Value& dst, const ir::Value& src,
                                u32 shift) = 0;
    virtual void EmitCast(const ir::Value& dst, const ir::Value& src,
                          DataType dst_type) = 0;
    virtual void EmitCompare(const ir::Value& dst, ComparisonOp op,
                             const ir::Value& srcA, const ir::Value& srcB) = 0;
    virtual void EmitBitwise(const ir::Value& dst, BitwiseOp op,
                             const ir::Value& srcA, const ir::Value& srcB) = 0;
    virtual void EmitSelect(const ir::Value& dst, const ir::Value& cond,
                            const ir::Value& src_true,
                            const ir::Value& src_false) = 0;

    // Control flow
    virtual void EmitBranch(label_t target) = 0;
    virtual void EmitBranchConditional(const ir::Value& cond,
                                       label_t target_true,
                                       label_t target_false) = 0;
    virtual void EmitBeginIf(const ir::Value& cond) = 0;
    virtual void EmitEndIf() = 0;

    // Math
    virtual void EmitRound(const ir::Value& dst, const ir::Value& src) = 0;
    virtual void EmitFloor(const ir::Value& dst, const ir::Value& src) = 0;
    virtual void EmitCeil(const ir::Value& dst, const ir::Value& src) = 0;
    virtual void EmitTrunc(const ir::Value& dst, const ir::Value& src) = 0;
    virtual void EmitMin(const ir::Value& dst, const ir::Value& srcA,
                         const ir::Value& srcB) = 0;
    virtual void EmitMax(const ir::Value& dst, const ir::Value& srcA,
                         const ir::Value& srcB) = 0;
    virtual void EmitClamp(const ir::Value& dst, const ir::Value& srcA,
                           const ir::Value& srcB, const ir::Value& srcC) = 0;
    virtual void EmitMathFunction(const ir::Value& dst, MathFunc func,
                                  const ir::Value& src) = 0;

    // Vector
    virtual void EmitVectorExtract(const ir::Value& dst, const ir::Value& src,
                                   u32 index) = 0;
    virtual void EmitVectorInsert(const ir::Value& dst, const ir::Value& src,
                                  u32 index) = 0;
    virtual void
    EmitVectorConstruct(const ir::Value& dst, DataType data_type,
                        const std::vector<ir::Value>& elements) = 0;

    // Special
    virtual void EmitExit() = 0;
    virtual void EmitDiscard() = 0;
    virtual void EmitTextureSample(const ir::Value& dst, u32 const_buffer_index,
                                   const ir::Value& coords) = 0;
    virtual void EmitTextureRead(const ir::Value& dst, u32 const_buffer_index,
                                 const ir::Value& coords) = 0;
    virtual void EmitTextureQueryDimension(const ir::Value& dst,
                                           u32 const_buffer_index,
                                           u32 dimension) = 0;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen
