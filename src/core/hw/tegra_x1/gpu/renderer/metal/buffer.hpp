#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Buffer final : public BufferBase {
  public:
    Buffer(u64 size);
    Buffer(MTL::Buffer* buffer_);
    ~Buffer() override;

    uptr GetPtr() const override {
        return reinterpret_cast<uptr>(buffer->contents());
    }

    // Copying
    void CopyFrom(ICommandBuffer* command_buffer, TextureBase* src,
                  const uint3 src_origin, const uint3 src_size,
                  u64 dst_offset) override;

  private:
    MTL::Buffer* buffer;

    // Copying
    void CopyFromImpl(const uptr data, u64 dst_offset, u64 size_) override;
    void CopyFromImpl(ICommandBuffer* command_buffer, BufferBase* src,
                      u64 dst_offset, u64 src_offset, u64 size_) override;

  public:
    GETTER(buffer, GetBuffer);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
