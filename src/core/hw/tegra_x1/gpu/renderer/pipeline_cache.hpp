#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class PipelineBase;

class PipelineCache
    : public CacheBase<PipelineCache, PipelineBase*, PipelineDescriptor> {
  public:
    void Destroy() {}

    PipelineBase* Create(const PipelineDescriptor& descriptor);
    void Update(PipelineBase* pipeline) {}
    u64 Hash(const PipelineDescriptor& descriptor);

    void DestroyElement(PipelineBase* pipeline);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
