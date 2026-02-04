#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class BufferBase;

class TextureBase {
  public:
    TextureBase(const TextureDescriptor& descriptor_)
        : descriptor{descriptor_} {}
    virtual ~TextureBase() = default;

    virtual TextureBase*
    CreateView(const TextureViewDescriptor& descriptor) = 0;

    // Copying
    virtual void CopyFrom(const uptr data) = 0;
    virtual void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src,
                          const usize src_stride, const uint3 dst_origin,
                          const usize3 size) = 0;
    void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src) {
        CopyFrom(command_buffer, src, descriptor.stride, uint3({0, 0, 0}),
                 usize3({descriptor.width, descriptor.height, 1}));
    }
    virtual void CopyFrom(ICommandBuffer* command_buffer,
                          const TextureBase* src, const uint3 src_origin,
                          const uint3 dst_origin, const usize3 size) = 0;

    // Blitting
    virtual void BlitFrom(ICommandBuffer* command_buffer,
                          const TextureBase* src, const float3 src_origin,
                          const usize3 src_size, const float3 dst_origin,
                          const usize3 dst_size) = 0;

    // Getters
    const TextureDescriptor& GetDescriptor() const { return descriptor; }

  protected:
    const TextureDescriptor descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
