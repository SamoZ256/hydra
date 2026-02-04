#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

struct BlitParams {
    float2 src_offset;
    float2 src_scale;
    f32 opacity;
};

struct BlitPipelineDescriptor {
    MTL::PixelFormat pixel_format;
    bool transparent;
};

class BlitPipelineCache
    : public CacheBase<BlitPipelineCache, MTL::RenderPipelineState*,
                       BlitPipelineDescriptor> {
  public:
    BlitPipelineCache();

    void Destroy();

    MTL::RenderPipelineState* Create(const BlitPipelineDescriptor& descriptor);
    void Update([[maybe_unused]] MTL::RenderPipelineState* pipeline) {}
    u32 Hash(const BlitPipelineDescriptor& descriptor);

    void DestroyElement(MTL::RenderPipelineState* pipeline);

  private:
    MTL::RenderPipelineDescriptor* pipeline_descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
