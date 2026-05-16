#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class SamplerBase;
class IRenderer;

class SamplerCache
    : public CacheBase<SamplerCache, SamplerBase*, SamplerDescriptor> {
  public:
    SamplerCache(IRenderer& renderer_) : renderer{renderer_} {}

    void Destroy() {}

    SamplerBase* Create(const SamplerDescriptor& descriptor);
    void Update([[maybe_unused]] SamplerBase* sampler) {}
    u32 Hash(const SamplerDescriptor& descriptor);

    void DestroyElement(SamplerBase* sampler);

  private:
    IRenderer& renderer;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
