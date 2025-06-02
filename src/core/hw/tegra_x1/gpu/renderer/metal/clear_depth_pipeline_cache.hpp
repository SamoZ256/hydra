#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

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

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
