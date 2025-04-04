#pragma once

#include "hw/tegra_x1/gpu/renderer/const.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer {

class BufferBase;

class TextureBase {
  public:
    TextureBase(const TextureDescriptor& descriptor_)
        : descriptor{descriptor_} {}
    virtual ~TextureBase() = default;

    // Copying
    virtual void CopyFrom(const void* data) = 0;
    virtual void CopyFrom(const BufferBase* src, const usize src_stride,
                          const u32 dst_layer, const uint3 dst_origin,
                          const usize3 size) = 0;
    // virtual void CopyFrom(const TextureBase* texture) = 0;

    // Getters
    const TextureDescriptor& GetDescriptor() const { return descriptor; }

  protected:
    const TextureDescriptor descriptor;
};

} // namespace Hydra::HW::TegraX1::GPU::Renderer
