#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/iterator_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/lang/structurizer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang {

class LangBuilderBase;

class StructuredIterator : public IteratorBase {
  public:
    StructuredIterator(Reader code_reader, const CfgBlock* root_block_)
        : IteratorBase(code_reader), root_block{root_block_} {}

    void Iterate(ObserverBase* observer) override;

  private:
    const CfgBlock* root_block;

    void IterateImpl(LangBuilderBase* builder, const CfgNode* node);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Lang
