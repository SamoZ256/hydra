#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::cpu {
class MMUBase;
}

namespace hydra::hw::tegra_x1::gpu::renderer {

class BufferBase;

class BufferCache
    : public CacheBase<BufferCache, BufferBase*, BufferDescriptor> {
  public:
    void Destroy() {}

    BufferBase* Create(const BufferDescriptor& descriptor);
    void Update(BufferBase* buffer);
    u64 Hash(const BufferDescriptor& descriptor);

    void DestroyElement(BufferBase* buffer);

  private:
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
