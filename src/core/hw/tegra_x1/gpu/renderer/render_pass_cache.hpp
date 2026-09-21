#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class RenderPassBase;
class IRenderer;

class RenderPassCache
    : public CacheBase<RenderPassCache, RenderPassBase*, RenderPassDescriptor> {
  public:
    explicit RenderPassCache(IRenderer& renderer_) : renderer{renderer_} {}

    void destroy() {}

    RenderPassBase* create(const RenderPassDescriptor& descriptor);
    void update([[maybe_unused]] RenderPassBase* render_pass) {}
    static u32 hash(const RenderPassDescriptor& descriptor);

    static void destroyElement(RenderPassBase* render_pass);

  private:
    IRenderer& renderer;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
