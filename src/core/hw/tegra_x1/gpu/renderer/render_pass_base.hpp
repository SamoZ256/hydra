#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class RenderPassBase {
  public:
    RenderPassBase(const RenderPassDescriptor& descriptor_)
        : descriptor{descriptor_} {}
    virtual ~RenderPassBase() = default;

    // Getters
    const RenderPassDescriptor& GetDescriptor() const { return descriptor; }

  protected:
    const RenderPassDescriptor descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
