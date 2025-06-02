#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

struct DepthStencilStateDescriptor {
    bool depth_test_enabled;
    bool depth_write_enabled;
    engines::DepthTestFunc depth_test_func;
    // TODO: stencil
};

class DepthStencilStateCache
    : public CacheBase<DepthStencilStateCache, MTL::DepthStencilState*,
                       DepthStencilStateDescriptor> {
  public:
    void Destroy() {}

    MTL::DepthStencilState*
    Create(const DepthStencilStateDescriptor& descriptor);
    void Update(MTL::DepthStencilState* depth_stencil_state) {}
    u64 Hash(const DepthStencilStateDescriptor& descriptor);

    void DestroyElement(MTL::DepthStencilState* depth_stencil_state);

  private:
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
