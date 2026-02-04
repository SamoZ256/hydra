#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class BufferBase;

class TextureBase {
  public:
    TextureBase(const TextureDescriptor& descriptor_)
        : descriptor{descriptor_} {}
    virtual ~TextureBase() = default;

    virtual TextureBase*
    CreateView(const TextureViewDescriptor& descriptor) = 0;

    // Copying
    virtual void CopyFrom(const BufferBase* src, const usize src_stride,
                          const uint3 dst_origin, const usize3 size) = 0;
    void CopyFrom(const BufferBase* src) {
        CopyFrom(src, descriptor.stride, uint3({0, 0, 0}),
                 usize3({descriptor.width, descriptor.height, 1}));
    }
    virtual void CopyFrom(const TextureBase* src, const uint3 src_origin,
                          const uint3 dst_origin, const usize3 size) = 0;

    // Blitting
    virtual void BlitFrom(const TextureBase* src, const float3 src_origin,
                          const usize3 src_size, const float3 dst_origin,
                          const usize3 dst_size) = 0;

    // Getters
    const TextureDescriptor& GetDescriptor() const { return descriptor; }

  protected:
    const TextureDescriptor descriptor;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
