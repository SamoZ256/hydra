#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/surface_compositor.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class CommandBuffer;
class Renderer;

class SurfaceCompositor final : public ISurfaceCompositor {
  public:
    SurfaceCompositor(Renderer& renderer_, CA::MetalDrawable* drawable_);
    ~SurfaceCompositor() override;

    void DrawTexture(ICommandBuffer* command_buffer,
                     const ITextureView* texture, const FloatRect2D src_rect,
                     const FloatRect2D dst_rect, bool transparent,
                     f32 opacity) override;
    void Present(ICommandBuffer* command_buffer) override;

  private:
    Renderer& renderer;
    CA::MetalDrawable* drawable;
    MTL::RenderPassDescriptor* render_pass_descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
