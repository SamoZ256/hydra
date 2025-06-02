#pragma once

#include "core/hw/tegra_x1/gpu/engines/inline_base.hpp"

namespace hydra::hw::tegra_x1::gpu::engines {

struct RegsCompute {
    RegsInline regs_inline;

    u32 padding_0x6e[0x915];
};

class Compute : public EngineWithRegsBase<RegsCompute>, public InlineBase {
  public:
    void Method(u32 method, u32 arg) override;

  private:
    // Methods
    DEFINE_INLINE_ENGINE_METHODS;
};

} // namespace hydra::hw::tegra_x1::gpu::engines
