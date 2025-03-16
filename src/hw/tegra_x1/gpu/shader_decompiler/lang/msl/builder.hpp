#pragma once

#include "hw/tegra_x1/gpu/shader_decompiler/lang/lang_builder_base.hpp"

namespace Hydra::HW::TegraX1::GPU::ShaderDecompiler::Lang::MSL {

class Builder final : public LangBuilderBase {
  public:
    Builder(const Analyzer& analyzer, std::vector<u8>& out_code)
        : LangBuilderBase(analyzer, out_code) {}

  protected:
    void EmitHeader() override;
    void EmitTypeAliases() override;

    std::string GetQualifierName(const Qualifier qualifier) override;

  private:
};

} // namespace Hydra::HW::TegraX1::GPU::ShaderDecompiler::Lang::MSL
