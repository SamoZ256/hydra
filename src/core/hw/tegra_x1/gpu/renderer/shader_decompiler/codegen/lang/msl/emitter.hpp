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

    void emitHeader() override;
    void emitTypeAliases() override;
    void emitDeclarations() override;
    void emitStateBindings() override;
    void emitStateBindingAssignments() override;
    void emitMainPrototype() override;
    void emitExitReturn() override;

    // Data
    void emitBitCast(const ir::Value& dst, const ir::Value& src) override;

    // Math
    void emitIsNan(const ir::Value& dst, const ir::Value& src) override;
    void emitReciprocal(const ir::Value& dst, const ir::Value& src) override;
    void emitSin(const ir::Value& dst, const ir::Value& src) override;
    void emitCos(const ir::Value& dst, const ir::Value& src) override;
    void emitExp2(const ir::Value& dst, const ir::Value& src) override;
    void emitLog2(const ir::Value& dst, const ir::Value& src) override;
    void emitSqrt(const ir::Value& dst, const ir::Value& src) override;
    void emitReciprocalSqrt(const ir::Value& dst,
                            const ir::Value& src) override;

    // Logical & Bitwise
    void emitBitfieldExtract(const ir::Value& dst, const ir::Value& src_a,
                             const ir::Value& src_b,
                             const ir::Value& src_c) override;

    // Texture
    void emitTextureSample(const ir::Value& dst, u32 const_buffer_index,
                           TextureType type, TextureSampleFlags flags,
                           const ir::Value& array_index,
                           const ir::Value& coords, const ir::Value& cmp_value,
                           const ir::Value& lod) override;
    void emitTextureGather(const ir::Value& dst, u32 const_buffer_index,
                           const ir::Value& coords, u8 component) override;
    void emitTextureQueryDimension(const ir::Value& dst, u32 const_buffer_index,
                                   u32 dimension) override;

    // Exit
    void emitDiscard() override;

    std::string getSvAccessQualifiedStr(const SvAccess& sv_access,
                                        bool output) override;

  private:
    // Helpers
    static std::string getSvStr(const Sv& sv);
    std::string getSvQualifierStr(const Sv& sv, bool output);
};

} // namespace
  // hydra::hw::tegra_x1::gpu::renderer::shader_decomp::codegen::lang::msl
