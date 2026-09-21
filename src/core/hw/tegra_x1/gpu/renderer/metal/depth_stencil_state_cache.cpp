#include "core/hw/tegra_x1/gpu/renderer/metal/depth_stencil_state_cache.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

MTL::DepthStencilState*
DepthStencilStateCache::create(const DepthStencilStateDescriptor& descriptor) {
    auto desc = MTL::DepthStencilDescriptor::alloc()->init();

    if (descriptor.depth_test_enabled)
        desc->setDepthCompareFunction(
            toMtlCompareFunc(descriptor.depth_compare_op));

    // HACK: Minecraft: Story Mode overrides the depth buffer
    if (descriptor.depth_write_enabled && descriptor.depth_test_enabled &&
        descriptor.depth_compare_op != engines::CompareOp::Always)
        desc->setDepthWriteEnabled(true);

    return device->newDepthStencilState(desc);
}

u32 DepthStencilStateCache::hash(
    const DepthStencilStateDescriptor& descriptor) {
    ztd::hash::XxHash32 hash;
    hash.add(descriptor.depth_test_enabled);
    hash.add(descriptor.depth_write_enabled);
    hash.add(descriptor.depth_compare_op);
    return hash.toHashCode();
}

void DepthStencilStateCache::destroyElement(
    MTL::DepthStencilState* depth_stencil_state) {
    depth_stencil_state->release();
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
