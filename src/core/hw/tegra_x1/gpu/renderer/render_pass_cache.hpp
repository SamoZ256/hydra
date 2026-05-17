#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class RenderPassBase;
class IRenderer;

class RenderPassCache
    : public CacheBase<RenderPassCache, RenderPassBase*, RenderPassDescriptor> {
  public:
    RenderPassCache(IRenderer& renderer_) : renderer{renderer_} {}

    void Destroy() {}

    RenderPassBase* Create(const RenderPassDescriptor& descriptor);
    void Update([[maybe_unused]] RenderPassBase* render_pass) {}
    u32 Hash(const RenderPassDescriptor& descriptor);

    void DestroyElement(RenderPassBase* render_pass);

  private:
    IRenderer& renderer;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
