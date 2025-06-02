#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class SamplerBase {
  public:
    SamplerBase(const SamplerDescriptor& descriptor_)
        : descriptor{descriptor_} {}
    virtual ~SamplerBase() = default;

    // Getters
    const SamplerDescriptor& GetDescriptor() const { return descriptor; }

  protected:
    const SamplerDescriptor descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
