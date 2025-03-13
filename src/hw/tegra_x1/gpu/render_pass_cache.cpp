#include "hw/tegra_x1/gpu/render_pass_cache.hpp"

#include "hw/tegra_x1/gpu/gpu.hpp"
#include "hw/tegra_x1/gpu/renderer/render_pass_base.hpp"

// HACK
template <typename T> T rotl(T v, u64 shift) {
    return (v << shift) | (v >> (32 - shift));
}

namespace Hydra::HW::TegraX1::GPU {

Renderer::RenderPassBase*
RenderPassCache::Create(const Renderer::RenderPassDescriptor& descriptor) {
    auto render_pass = RENDERER->CreateRenderPass(descriptor);

    return render_pass;
}

u64 RenderPassCache::Hash(const Renderer::RenderPassDescriptor& descriptor) {
    // TODO: improve this
    // TODO: also hash metadata about clears
    u64 hash = 0;
    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++)
        hash ^= reinterpret_cast<u64>(descriptor.color_targets[i].texture);
    hash ^= reinterpret_cast<u64>(descriptor.depth_stencil_target.texture);

    return hash;
}

void RenderPassCache::DestroyElement(Renderer::RenderPassBase* render_pass) {
    delete render_pass;
}

} // namespace Hydra::HW::TegraX1::GPU
