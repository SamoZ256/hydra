#include "core/hw/tegra_x1/gpu/renderer/metal/depth_stencil_state_cache.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

MTL::DepthStencilState*
DepthStencilStateCache::Create(const DepthStencilStateDescriptor& descriptor) {
    auto desc = MTL::DepthStencilDescriptor::alloc()->init();

    if (descriptor.depth_test_enabled)
        desc->setDepthCompareFunction(
            to_mtl_compare_func(descriptor.depth_compare_op));

    // HACK: Minecraft: Story Mode overrides the depth buffer
    if (descriptor.depth_write_enabled && descriptor.depth_test_enabled &&
        descriptor.depth_compare_op != engines::CompareOp::Always)
        desc->setDepthWriteEnabled(true);

    return device->newDepthStencilState(desc);
}

u32 DepthStencilStateCache::Hash(
    const DepthStencilStateDescriptor& descriptor) {
    HashCode hash;
    hash.Add(descriptor.depth_test_enabled);
    hash.Add(descriptor.depth_write_enabled);
    hash.Add(descriptor.depth_compare_op);
    return hash.ToHashCode();
}

void DepthStencilStateCache::DestroyElement(
    MTL::DepthStencilState* depth_stencil_state) {
    depth_stencil_state->release();
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
