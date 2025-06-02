#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/lang_builder_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang::MSL {

class Builder final : public LangBuilderBase {
  public:
    using LangBuilderBase::LangBuilderBase;

    void InitializeResourceMapping() override;

    // Operations
    void OpDiscard() override;

  protected:
    void EmitHeader() override;
    void EmitTypeAliases() override;
    void EmitDeclarations() override;
    void EmitMainPrototype() override;
    void EmitExit() override;

    std::string EmitTextureSample(u32 const_buffer_index,
                                  const std::string_view coords) override;

  private:
    // Helpers
    std::string GetSvQualifierName(const Sv& sv, bool output);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang::MSL
