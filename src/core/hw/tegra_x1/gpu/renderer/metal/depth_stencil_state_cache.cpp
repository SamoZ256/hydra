#include "core/hw/tegra_x1/gpu/renderer/metal/depth_stencil_state_cache.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

MTL::DepthStencilState*
DepthStencilStateCache::Create(const DepthStencilStateDescriptor& descriptor) {
    auto desc = MTL::DepthStencilDescriptor::alloc()->init();

    if (descriptor.depth_test_enabled)
        desc->setDepthCompareFunction(
            to_mtl_compare_func(descriptor.depth_test_func));

    if (descriptor.depth_write_enabled)
        desc->setDepthWriteEnabled(true);

    return METAL_RENDERER_INSTANCE.GetDevice()->newDepthStencilState(desc);
}

u64 DepthStencilStateCache::Hash(
    const DepthStencilStateDescriptor& descriptor) {
    u64 hash = 0;
    hash += descriptor.depth_test_enabled;
    hash = rotl(hash, 1);
    hash += descriptor.depth_write_enabled;
    hash = rotl(hash, 1);
    hash += (u64)descriptor.depth_test_func;
    hash = rotl(hash, 5);

    return hash;
}

void DepthStencilStateCache::DestroyElement(
    MTL::DepthStencilState* depth_stencil_state) {
    depth_stencil_state->release();
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
