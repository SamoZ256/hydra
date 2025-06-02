#include "core/hw/tegra_x1/gpu/renderer/metal/render_pass.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

RenderPass::RenderPass(const RenderPassDescriptor& descriptor)
    : RenderPassBase(descriptor) {
    render_pass_descriptor = MTL::RenderPassDescriptor::alloc()->init();

    // Color targets
    for (u32 i = 0; i < COLOR_TARGET_COUNT; i++) {
        const auto& color_target = descriptor.color_targets[i];
        if (!color_target.texture)
            continue;

        auto color_attachment =
            render_pass_descriptor->colorAttachments()->object(i);
        color_attachment->setTexture(
            static_cast<Texture*>(descriptor.color_targets[i].texture)
                ->GetTexture());
        if (color_target.load_action_clear) {
            color_attachment->setLoadAction(MTL::LoadActionClear);
            color_attachment->setClearColor(
                MTL::ClearColor(color_target.clear_data.color[0],
                                color_target.clear_data.color[1],
                                color_target.clear_data.color[2],
                                color_target.clear_data.color[3]));
        } else {
            color_attachment->setLoadAction(MTL::LoadActionLoad);
        }
        color_attachment->setStoreAction(MTL::StoreActionStore);
    }

    // Depth stencil target
    if (descriptor.depth_stencil_target.texture) {
        const auto& depth_stencil_target = descriptor.depth_stencil_target;

        // Depth
        auto depth_attachment = render_pass_descriptor->depthAttachment();
        depth_attachment->setTexture(
            static_cast<Texture*>(depth_stencil_target.texture)->GetTexture());
        if (depth_stencil_target.load_action_clear &&
            depth_stencil_target.clear_data.clear_depth) {
            depth_attachment->setLoadAction(MTL::LoadActionClear);
            depth_attachment->setClearDepth(
                depth_stencil_target.clear_data.depth);
        } else {
            depth_attachment->setLoadAction(MTL::LoadActionLoad);
        }
        depth_attachment->setStoreAction(MTL::StoreActionStore);

        // Stencil
        // TODO: if format has stencil
        if (false) {
            auto stencil_attachment =
                render_pass_descriptor->stencilAttachment();
            stencil_attachment->setTexture(
                static_cast<Texture*>(depth_stencil_target.texture)
                    ->GetTexture());
            if (depth_stencil_target.load_action_clear &&
                depth_stencil_target.clear_data.clear_stencil) {
                stencil_attachment->setLoadAction(MTL::LoadActionClear);
                stencil_attachment->setClearStencil(
                    depth_stencil_target.clear_data.stencil);
            } else {
                stencil_attachment->setLoadAction(MTL::LoadActionLoad);
            }
            stencil_attachment->setStoreAction(MTL::StoreActionStore);
        }
    }
}

RenderPass::~RenderPass() { render_pass_descriptor->release(); }

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
