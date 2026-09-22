#include "core/hw/tegra_x1/gpu/renderer/metal/surface_compositor.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/blit_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

SurfaceCompositor::SurfaceCompositor(Renderer& renderer_,
                                     CA::MetalDrawable* drawable_)
    : renderer{renderer_}, drawable{drawable_} {
    // Render pass
    render_pass_descriptor = MTL::RenderPassDescriptor::alloc()->init();
    auto color_attachment =
        render_pass_descriptor->colorAttachments()->object(0);
    color_attachment->setTexture(drawable->texture());
    color_attachment->setLoadAction(MTL::LoadActionClear);
    color_attachment->setClearColor(MTL::ClearColor::Make(0.0, 0.0, 0.0, 1.0));
    color_attachment->setStoreAction(MTL::StoreActionStore);
}

SurfaceCompositor::~SurfaceCompositor() { render_pass_descriptor->release(); }

void SurfaceCompositor::drawTexture(ICommandBuffer* command_buffer,
                                    const ITextureView* texture,
                                    const FloatRect2D src_rect,
                                    const FloatRect2D dst_rect,
                                    bool transparent, f32 opacity) {
    auto command_buffer_impl = static_cast<CommandBuffer*>(command_buffer);
    auto texture_impl = static_cast<const TextureView*>(texture);

    auto encoder =
        command_buffer_impl->getRenderCommandEncoder(render_pass_descriptor);

    // Draw
    encoder->setRenderPipelineState(renderer.getBlitPipelineCache().find(
        {.pixel_format = drawable->texture()->pixelFormat(),
         .transparent = transparent}));
    encoder->setViewport(
        MTL::Viewport{.originX = static_cast<f64>(dst_rect.origin.x()),
                      .originY = static_cast<f64>(dst_rect.origin.y()),
                      .width = static_cast<f64>(dst_rect.size.x()),
                      .height = static_cast<f64>(dst_rect.size.y()),
                      .znear = 0.0,
                      .zfar = 1.0});

    u32 zero = 0;
    encoder->setVertexBytes(&zero, sizeof(zero), 0);

    // Src rect
    const auto& descriptor = texture->getBase()->getDescriptor();
    const auto src_width = descriptor.width;
    const auto src_height = descriptor.height;
    BlitParams params = {
        .src_offset = {src_rect.origin.x() / static_cast<f32>(src_width),
                       src_rect.origin.y() / static_cast<f32>(src_height)},
        .src_scale = {src_rect.size.x() / static_cast<f32>(src_width),
                      src_rect.size.y() / static_cast<f32>(src_height)},
        .opacity = opacity,
    };

    encoder->setFragmentBytes(&params, sizeof(params), 0);
    encoder->setFragmentTexture(texture_impl->getTexture(),
                                static_cast<NS::UInteger>(0));
    encoder->setFragmentSamplerState(renderer.getLinearSampler(),
                                     static_cast<NS::UInteger>(0));
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle,
                            static_cast<NS::UInteger>(0),
                            static_cast<NS::UInteger>(3));
}

void SurfaceCompositor::present(ICommandBuffer* command_buffer) {
    auto command_buffer_impl = static_cast<CommandBuffer*>(command_buffer);

    command_buffer_impl->getRenderCommandEncoder(render_pass_descriptor);
    command_buffer_impl->endEncoding();
    command_buffer_impl->getCommandBuffer()->presentDrawable(drawable);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
