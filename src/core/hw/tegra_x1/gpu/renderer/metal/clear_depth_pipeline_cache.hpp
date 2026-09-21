#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class ClearDepthPipelineCache
    : public CacheBase<ClearDepthPipelineCache, MTL::RenderPipelineState*,
                       MTL::PixelFormat> {
  public:
    explicit ClearDepthPipelineCache(MTL::Device* device_);

    void destroy();

    MTL::RenderPipelineState* create(MTL::PixelFormat pixel_format);
    void update([[maybe_unused]] MTL::RenderPipelineState* pipeline) {}
    static u32 hash(MTL::PixelFormat pixel_format);

    static void destroyElement(MTL::RenderPipelineState* pipeline);

  private:
    MTL::Device* device;

    MTL::RenderPipelineDescriptor* pipeline_descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
