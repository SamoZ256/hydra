#pragma once

#include "hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

class ClearDepthPipelineCache
    : public CacheBase<ClearDepthPipelineCache, MTL::RenderPipelineState*,
                       MTL::PixelFormat> {
  public:
    ClearDepthPipelineCache();

    void Destroy();

    MTL::RenderPipelineState* Create(MTL::PixelFormat pixel_format);
    void Update(MTL::RenderPipelineState* pipeline) {}
    u64 Hash(MTL::PixelFormat pixel_format);

    void DestroyElement(MTL::RenderPipelineState* pipeline);

  private:
    MTL::RenderPipelineDescriptor* pipeline_descriptor;
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
