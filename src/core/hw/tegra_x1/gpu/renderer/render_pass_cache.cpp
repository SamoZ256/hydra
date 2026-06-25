#include "core/hw/tegra_x1/gpu/renderer/render_pass_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

RenderPassBase*
RenderPassCache::Create(const RenderPassDescriptor& descriptor) {
    return renderer.CreateRenderPass(descriptor);
}

u32 RenderPassCache::Hash(const RenderPassDescriptor& descriptor) {
    ztd::hash::XxHash32 hash;

    // TODO: improve this
    // TODO: also hash metadata about clears
    for (const auto& color_target : descriptor.color_targets)
        hash.add(color_target.texture);
    hash.add(descriptor.depth_stencil_target.texture);

    return hash.toHashCode();
}

void RenderPassCache::DestroyElement(RenderPassBase* render_pass) {
    delete render_pass;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
