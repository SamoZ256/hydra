#pragma once

#include "core/hw/tegra_x1/gpu/renderer/shader_decompiler/observer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {
class GuestShaderState;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp::Analyzer {
class MemoryAnalyzer;
}

namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp {

class BuilderBase : public ObserverBase {
  public:
    BuilderBase(const Analyzer::MemoryAnalyzer& memory_analyzer_,
                const ShaderType type_, const GuestShaderState& state_,
                std::vector<u8>& out_code_,
                ResourceMapping& out_resource_mapping_)
        : memory_analyzer{memory_analyzer_}, type{type_}, state{state_},
          out_code{out_code_}, out_resource_mapping{out_resource_mapping_} {}
    virtual ~BuilderBase() {}

    virtual void InitializeResourceMapping() = 0;
    virtual void Start() = 0;
    virtual void Finish() = 0;

  protected:
    const Analyzer::MemoryAnalyzer& memory_analyzer;
    const ShaderType type;
    const GuestShaderState& state;

    std::vector<u8>& out_code;
    ResourceMapping& out_resource_mapping;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::shader_decomp
