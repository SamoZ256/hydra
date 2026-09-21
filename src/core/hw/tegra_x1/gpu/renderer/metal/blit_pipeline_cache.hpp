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
    explicit BlitPipelineCache(MTL::Device* device_);

    void destroy();

    MTL::RenderPipelineState* create(const BlitPipelineDescriptor& descriptor);
    void update([[maybe_unused]] MTL::RenderPipelineState* pipeline) {}
    u32 hash(const BlitPipelineDescriptor& descriptor);

    void destroyElement(MTL::RenderPipelineState* pipeline);

  private:
    MTL::Device* device;

    MTL::RenderPipelineDescriptor* pipeline_descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
