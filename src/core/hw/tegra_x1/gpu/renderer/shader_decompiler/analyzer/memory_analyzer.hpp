#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/module.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

struct TextureInfo {
    TextureType type;
    bool is_depth;
};

class MemoryAnalyzer {
  public:
    void analyze(const ir::Module& modul);

  private:
    std::unordered_set<SvSemantic> input_svs;
    std::unordered_set<SvSemantic> output_svs;
    std::unordered_set<u8> stage_inputs;
    std::unordered_set<u8> stage_outputs;
    std::unordered_set<u32> const_buffers;
    std::map<u32, TextureInfo> textures;

    // Helpers
    void handleAMemLoad(const AMem amem);
    void handleCMemLoad(const CMem cmem);
    void handleAMemStore(const AMem amem);
    void handleTextureAccess(u32 const_buffer_index, const TextureInfo& info);

  public:
    CONST_REF_GETTER(input_svs, getInputSVs);
    CONST_REF_GETTER(output_svs, getOutputSVs);
    CONST_REF_GETTER(stage_inputs, getStageInputs);
    CONST_REF_GETTER(stage_outputs, getStageOutputs);
    CONST_REF_GETTER(const_buffers, getConstBuffers);
    CONST_REF_GETTER(textures, getTextures);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer
