#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/render_pass_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class RenderPass final : public RenderPassBase {
  public:
    explicit RenderPass(const RenderPassDescriptor& descriptor);
    ~RenderPass() override;

    // Getters
    MTL::RenderPassDescriptor* getRenderPassDescriptor() const {
        return render_pass_descriptor;
    }

  private:
    MTL::RenderPassDescriptor* render_pass_descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
