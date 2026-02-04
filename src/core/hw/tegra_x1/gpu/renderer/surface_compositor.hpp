#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class TextureBase;

class ISurfaceCompositor {
  public:
    virtual ~ISurfaceCompositor() = default;

    virtual void DrawTexture(ICommandBuffer* command_buffer,
                             const TextureBase* texture,
                             const FloatRect2D src_rect,
                             const FloatRect2D dst_rect, bool transparent,
                             f32 opacity = 1.0f) = 0;
    virtual void Present(ICommandBuffer* command_buffer) = 0;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
