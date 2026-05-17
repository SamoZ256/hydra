#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

struct DepthStencilStateDescriptor {
    bool depth_test_enabled;
    bool depth_write_enabled;
    engines::CompareOp depth_compare_op;
    // TODO: stencil
};

class DepthStencilStateCache
    : public CacheBase<DepthStencilStateCache, MTL::DepthStencilState*,
                       DepthStencilStateDescriptor> {
  public:
    DepthStencilStateCache(MTL::Device* device_) : device{device_} {}

    void Destroy() {}

    MTL::DepthStencilState*
    Create(const DepthStencilStateDescriptor& descriptor);
    void Update([[maybe_unused]] MTL::DepthStencilState* depth_stencil_state) {}
    u32 Hash(const DepthStencilStateDescriptor& descriptor);

    void DestroyElement(MTL::DepthStencilState* depth_stencil_state);

  private:
    MTL::Device* device;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
