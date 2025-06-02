#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ShaderBase {
  public:
    ShaderBase(const ShaderDescriptor& descriptor_) : descriptor(descriptor_) {}
    virtual ~ShaderBase() = default;

    // Getters
    const ShaderDescriptor& GetDescriptor() const { return descriptor; }

  private:
    const ShaderDescriptor descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
