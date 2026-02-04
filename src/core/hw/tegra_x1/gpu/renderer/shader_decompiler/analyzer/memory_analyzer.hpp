#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/ir/module.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer {

struct TextureInfo {
    TextureType type;
    bool is_depth;
};

class MemoryAnalyzer {
  public:
    void Analyze(const ir::Module& modul);

  private:
    std::unordered_set<SvSemantic> input_svs;
    std::unordered_set<SvSemantic> output_svs;
    std::unordered_set<u8> stage_inputs;
    std::unordered_set<u8> stage_outputs;
    std::unordered_set<u32> const_buffers;
    std::map<u32, TextureInfo> textures;

    // Helpers
    void HandleAMemLoad(const AMem amem);
    void HandleCMemLoad(const CMem cmem);
    void HandleAMemStore(const AMem amem);
    void HandleTextureAccess(u32 const_buffer_index, const TextureInfo& info);

  public:
    CONST_REF_GETTER(input_svs, GetInputSVs);
    CONST_REF_GETTER(output_svs, GetOutputSVs);
    CONST_REF_GETTER(stage_inputs, GetStageInputs);
    CONST_REF_GETTER(stage_outputs, GetStageOutputs);
    CONST_REF_GETTER(const_buffers, GetConstBuffers);
    CONST_REF_GETTER(textures, GetTextures);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::analyzer
