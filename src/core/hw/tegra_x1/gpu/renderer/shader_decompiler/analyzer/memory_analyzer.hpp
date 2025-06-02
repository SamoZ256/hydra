#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/observer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer {

class MemoryAnalyzer : public ObserverBase {
  public:
    // Operations

    // Value
    ValueBase* OpAttributeMemory(bool load, const AMem& amem,
                                 DataType data_type = DataType::U32,
                                 bool neg = false) override;
    ValueBase* OpConstMemoryL(const CMem& cmem,
                              DataType data_type = DataType::U32,
                              bool neg = false) override;

    // Special
    void OpTextureSample(ValueBase* dstA, ValueBase* dstB, ValueBase* dstC,
                         ValueBase* dstD, u32 const_buffer_index,
                         ValueBase* coords_x, ValueBase* coords_y) override;

    // Getters
    const std::vector<SvSemantic>& GetInputSVs() const { return input_svs; }
    const std::vector<SvSemantic>& GetOutputSVs() const { return output_svs; }
    const std::vector<u8>& GetStageInputs() const { return stage_inputs; }
    const std::vector<u8>& GetStageOutputs() const { return stage_outputs; }
    const std::map<u32, usize>& GetUniformBuffers() const {
        return uniform_buffers;
    }
    const std::vector<u32>& GetTextures() const { return textures; }

  private:
    std::vector<SvSemantic> input_svs;
    std::vector<SvSemantic> output_svs;
    std::vector<u8> stage_inputs;
    std::vector<u8> stage_outputs;
    std::map<u32, usize> uniform_buffers;
    std::vector<u32> textures;

    // Helpers
    void HandleAMemLoad(const AMem amem);
    void HandleCMemLoad(const CMem cmem);
    void HandleAMemStore(const AMem amem);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer
