#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Buffer final : public BufferBase {
  public:
    Buffer(const BufferDescriptor& descriptor);
    Buffer(MTL::Buffer* buffer_, u32 offset_);
    ~Buffer() override;

    // Copying
    void CopyFrom(const uptr data) override;
    void CopyFrom(BufferBase* src) override;

    // Getters
    MTL::Buffer* GetBuffer() const { return buffer; }

  private:
    MTL::Buffer* buffer;
    u32 offset{0};
    bool owns_buffer{true};
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
