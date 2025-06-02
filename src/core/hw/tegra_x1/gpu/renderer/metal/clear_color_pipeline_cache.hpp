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
    ClearColorPipelineCache();

    void Destroy();

    MTL::RenderPipelineState*
    Create(const ClearColorPipelineDescriptor& descriptor);
    void Update(MTL::RenderPipelineState* pipeline) {}
    u64 Hash(const ClearColorPipelineDescriptor& descriptor);

    void DestroyElement(MTL::RenderPipelineState* pipeline);

  private:
    MTL::RenderPipelineDescriptor* pipeline_descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
