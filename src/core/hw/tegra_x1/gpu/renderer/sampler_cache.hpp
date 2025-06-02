#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class MMUBase;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class SamplerBase;

class SamplerCache
    : public CacheBase<SamplerCache, SamplerBase*, SamplerDescriptor> {
  public:
    void Destroy() {}

    SamplerBase* Create(const SamplerDescriptor& descriptor);
    void Update(SamplerBase* sampler) {}
    u64 Hash(const SamplerDescriptor& descriptor);

    void DestroyElement(SamplerBase* sampler);

  private:
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
