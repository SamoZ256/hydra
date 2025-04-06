#pragma once

#include "hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

struct BlitPipelineDescriptor {
    MTL::PixelFormat pixel_format;
};

class BlitPipelineCache
    : public CacheBase<BlitPipelineCache, MTL::RenderPipelineState*,
                       BlitPipelineDescriptor> {
  public:
    BlitPipelineCache();

    void Destroy();

    MTL::RenderPipelineState*
    Create(const BlitPipelineDescriptor& descriptor);
    void Update(MTL::RenderPipelineState* pipeline) {}
    u64 Hash(const BlitPipelineDescriptor& descriptor);

    void DestroyElement(MTL::RenderPipelineState* pipeline);

  private:
    MTL::RenderPipelineDescriptor* pipeline_descriptor;
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
