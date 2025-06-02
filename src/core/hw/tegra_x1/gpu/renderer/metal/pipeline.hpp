#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/pipeline_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Pipeline final : public PipelineBase {
  public:
    Pipeline(const PipelineDescriptor& descriptor);
    ~Pipeline() override;

    // Getters
    MTL::RenderPipelineState* GetPipeline() const { return pipeline; }

  private:
    MTL::RenderPipelineState* pipeline;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
