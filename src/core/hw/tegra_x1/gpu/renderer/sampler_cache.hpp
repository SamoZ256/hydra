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
    explicit SamplerCache(IRenderer& renderer_) : renderer{renderer_} {}

    void destroy() {}

    SamplerBase* create(const SamplerDescriptor& descriptor);
    void update([[maybe_unused]] SamplerBase* sampler) {}
    static u32 hash(const SamplerDescriptor& descriptor);

    static void destroyElement(SamplerBase* sampler);

  private:
    IRenderer& renderer;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
