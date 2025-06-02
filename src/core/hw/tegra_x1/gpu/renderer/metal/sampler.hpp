#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/sampler_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Sampler final : public SamplerBase {
  public:
    Sampler(const SamplerDescriptor& descriptor);
    ~Sampler() override;

    // Getters
    MTL::SamplerState* GetSampler() const { return mtl_sampler; }

  private:
    MTL::SamplerState* mtl_sampler;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
