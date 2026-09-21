#include "core/hw/tegra_x1/gpu/renderer/null/surface_compositor.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::null {

SurfaceCompositor::SurfaceCompositor() = default;
SurfaceCompositor::~SurfaceCompositor() = default;

void SurfaceCompositor::drawTexture(
    [[maybe_unused]] ICommandBuffer* command_buffer,
    [[maybe_unused]] const ITextureView* texture,
    [[maybe_unused]] const FloatRect2D src_rect,
    [[maybe_unused]] const FloatRect2D dst_rect,
    [[maybe_unused]] bool transparent, [[maybe_unused]] f32 opacity) {}

void SurfaceCompositor::present(
    [[maybe_unused]] ICommandBuffer* command_buffer) {}

} // namespace hydra::hw::tegra_x1::gpu::renderer::null