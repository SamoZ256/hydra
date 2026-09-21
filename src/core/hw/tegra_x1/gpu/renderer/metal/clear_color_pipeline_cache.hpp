#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

struct ClearColorPipelineDescriptor {
    MTL::PixelFormat pixel_format;
    u32 render_target_id;
    u8 mask;
};

class ClearColorPipelineCache
    : public CacheBase<ClearColorPipelineCache, MTL::RenderPipelineState*,
                       ClearColorPipelineDescriptor> {
  public:
    explicit ClearColorPipelineCache(MTL::Device* device_);

    void destroy();

    MTL::RenderPipelineState*
    create(const ClearColorPipelineDescriptor& descriptor);
    void update([[maybe_unused]] MTL::RenderPipelineState* pipeline) {}
    static u32 hash(const ClearColorPipelineDescriptor& descriptor);

    static void destroyElement(MTL::RenderPipelineState* pipeline);

  private:
    MTL::Device* device;

    MTL::RenderPipelineDescriptor* pipeline_descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
