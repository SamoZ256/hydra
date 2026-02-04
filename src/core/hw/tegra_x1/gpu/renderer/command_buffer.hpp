#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer {
  public:
    virtual ~ICommandBuffer() = default;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
