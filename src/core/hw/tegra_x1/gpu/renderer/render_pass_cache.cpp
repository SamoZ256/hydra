#include "core/hw/tegra_x1/gpu/renderer/render_pass_cache.hpp"

#include "core/hw/tegra_x1/gpu/gpu.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

RenderPassBase*
RenderPassCache::Create(const RenderPassDescriptor& descriptor) {
    return RENDERER_INSTANCE->CreateRenderPass(descriptor);
}

u64 RenderPassCache::Hash(const RenderPassDescriptor& descriptor) {
    // TODO: improve this
    // TODO: also hash metadata about clears
    u64 hash = 0;
    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++)
        hash ^= reinterpret_cast<u64>(descriptor.color_targets[i].texture);
    hash ^= reinterpret_cast<u64>(descriptor.depth_stencil_target.texture);

    return hash;
}

void RenderPassCache::DestroyElement(RenderPassBase* render_pass) {
    delete render_pass;
}

} // namespace hydra::hw::tegra_x1::gpu::renderer
