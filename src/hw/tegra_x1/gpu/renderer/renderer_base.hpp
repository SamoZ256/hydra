#pragma once

#include "hw/tegra_x1/gpu/renderer/const.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer {

class TextureBase;
class RenderPassBase;

class RendererBase {
  public:
    virtual ~RendererBase() {}

    virtual void SetSurface(void* surface) = 0;

    virtual void Present(TextureBase* texture) = 0;

    // Texture
    virtual TextureBase* CreateTexture(const TextureDescriptor& descriptor) = 0;
    virtual void UploadTexture(TextureBase* texture, void* data) = 0;

    // Render pass
    virtual RenderPassBase*
    CreateRenderPass(const RenderPassDescriptor& descriptor) = 0;

  private:
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer
