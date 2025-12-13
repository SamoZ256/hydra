#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/codegen/lang/emitter.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::
    msl {

class MslEmitter final : public LangEmitter {
  public:
    MslEmitter(const DecompilerContext& context,
               const analyzer::MemoryAnalyzer& memory_analyzer,
               const GuestShaderState& state, std::vector<u8>& out_code,
               ResourceMapping& out_resource_mapping);

  protected:
    // Emit

    void EmitHeader() override;
    void EmitTypeAliases() override;
    void EmitDeclarations() override;
    void EmitStateBindings() override;
    void EmitStateBindingAssignments() override;
    void EmitMainPrototype() override;
    void EmitExitReturn() override;

    // Math
    void EmitIsNan(const ir::Value& dst, const ir::Value& src) override;
    void EmitReciprocal(const ir::Value& dst, const ir::Value& src) override;
    void EmitSin(const ir::Value& dst, const ir::Value& src) override;
    void EmitCos(const ir::Value& dst, const ir::Value& src) override;
    void EmitExp2(const ir::Value& dst, const ir::Value& src) override;
    void EmitLog2(const ir::Value& dst, const ir::Value& src) override;
    void EmitSqrt(const ir::Value& dst, const ir::Value& src) override;
    void EmitReciprocalSqrt(const ir::Value& dst,
                            const ir::Value& src) override;

    // Texture
    void EmitTextureSample(const ir::Value& dst, u32 const_buffer_index,
                           const ir::Value& coords) override;
    void EmitTextureRead(const ir::Value& dst, u32 const_buffer_index,
                         const ir::Value& coords) override;
    void EmitTextureGather(const ir::Value& dst, u32 const_buffer_index,
                           const ir::Value& coords, u8 component) override;
    void EmitTextureQueryDimension(const ir::Value& dst, u32 const_buffer_index,
                                   u32 dimension) override;

    // Exit
    void EmitDiscard() override;

    std::string GetSvAccessQualifiedStr(const SvAccess& sv_access,
                                        bool output) override;

  private:
    // Helpers
    std::string GetSvStr(const Sv& sv);
    std::string GetSvQualifierStr(const Sv& sv, bool output);
};

} // namespace
  // hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::msl
