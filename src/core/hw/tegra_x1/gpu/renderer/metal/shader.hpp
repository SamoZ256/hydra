#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/shader_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Shader final : public ShaderBase {
  public:
    Shader(const ShaderDescriptor& descriptor);
    ~Shader() override;

    // Getters
    MTL::Function* GetFunction() const { return function; }

  private:
    MTL::Function* function;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
