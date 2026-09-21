#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class PipelineBase {
  public:
    explicit PipelineBase(const PipelineDescriptor& descriptor_)
        : descriptor{descriptor_} {}
    virtual ~PipelineBase() = default;

    // Getters
    const PipelineDescriptor& getDescriptor() const { return descriptor; }

  protected:
    const PipelineDescriptor descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
