#include "core/hw/tegra_x1/gpu/renderer/render_pass_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

RenderPassBase*
RenderPassCache::Create(const RenderPassDescriptor& descriptor) {
    return renderer.CreateRenderPass(descriptor);
}

u32 RenderPassCache::Hash(const RenderPassDescriptor& descriptor) {
    HashCode hash;

    // TODO: improve this
    // TODO: also hash metadata about clears
    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++)
        hash.Add(descriptor.color_targets[i].texture);
    hash.Add(descriptor.depth_stencil_target.texture);

    return hash.ToHashCode();
}

void RenderPassCache::DestroyElement(RenderPassBase* render_pass) {
    delete render_pass;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
