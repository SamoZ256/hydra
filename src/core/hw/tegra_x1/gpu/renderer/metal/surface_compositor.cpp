#include "core/hw/tegra_x1/gpu/renderer/metal/surface_compositor.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/blit_pipeline_cache.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

SurfaceCompositor::SurfaceCompositor(CA::MetalDrawable* drawable_)
    : drawable{drawable_} {
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

void SurfaceCompositor::DrawTexture(ICommandBuffer* command_buffer,
                                    const TextureBase* texture,
                                    const FloatRect2D src_rect,
                                    const FloatRect2D dst_rect,
                                    bool transparent, f32 opacity) {
    auto command_buffer_impl = static_cast<CommandBuffer*>(command_buffer);
    auto texture_impl = static_cast<const Texture*>(texture);

    auto encoder =
        command_buffer_impl->GetRenderCommandEncoder(render_pass_descriptor);

    // Draw
    encoder->setRenderPipelineState(
        METAL_RENDERER_INSTANCE.GetBlitPipelineCache()->Find(
            {drawable->texture()->pixelFormat(), transparent}));
    encoder->setViewport(MTL::Viewport{
        (f64)dst_rect.origin.x(), (f64)dst_rect.origin.y(),
        (f64)dst_rect.size.x(), (f64)dst_rect.size.y(), 0.0, 1.0});

    u32 zero = 0;
    encoder->setVertexBytes(&zero, sizeof(zero), 0);

    // Src rect
    const auto src_width = texture->GetDescriptor().width;
    const auto src_height = texture->GetDescriptor().height;
    BlitParams params = {
        .src_offset = {src_rect.origin.x() / src_width,
                       src_rect.origin.y() / src_height},
        .src_scale = {src_rect.size.x() / src_width,
                      src_rect.size.y() / src_height},
        .opacity = opacity,
    };

    encoder->setFragmentBytes(&params, sizeof(params), 0);
    encoder->setFragmentTexture(texture_impl->GetTexture(), NS::UInteger(0));
    encoder->setFragmentSamplerState(METAL_RENDERER_INSTANCE.GetLinearSampler(),
                                     NS::UInteger(0));
    encoder->drawPrimitives(MTL::PrimitiveTypeTriangle, NS::UInteger(0),
                            NS::UInteger(3));
}

void SurfaceCompositor::Present(ICommandBuffer* command_buffer) {
    auto command_buffer_impl = static_cast<CommandBuffer*>(command_buffer);

    command_buffer_impl->GetRenderCommandEncoder(render_pass_descriptor);
    command_buffer_impl->EndEncoding();
    command_buffer_impl->GetCommandBuffer()->presentDrawable(drawable);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
