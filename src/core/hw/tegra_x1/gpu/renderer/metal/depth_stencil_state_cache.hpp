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
    explicit DepthStencilStateCache(MTL::Device* device_) : device{device_} {}

    void destroy() {}

    MTL::DepthStencilState*
    create(const DepthStencilStateDescriptor& descriptor);
    void update([[maybe_unused]] MTL::DepthStencilState* depth_stencil_state) {}
    static u32 hash(const DepthStencilStateDescriptor& descriptor);

    static void destroyElement(MTL::DepthStencilState* depth_stencil_state);

  private:
    MTL::Device* device;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
