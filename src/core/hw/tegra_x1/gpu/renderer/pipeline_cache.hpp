#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class PipelineBase;
class IRenderer;

class PipelineCache
    : public CacheBase<PipelineCache, PipelineBase*, PipelineDescriptor> {
  public:
    PipelineCache(IRenderer& renderer_) : renderer{renderer_} {}

    void Destroy() {}

    PipelineBase* Create(const PipelineDescriptor& descriptor);
    void Update([[maybe_unused]] PipelineBase* pipeline) {}
    u32 Hash(const PipelineDescriptor& descriptor);

    void DestroyElement(PipelineBase* pipeline);

  private:
    IRenderer& renderer;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
