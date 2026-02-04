#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Texture final : public TextureBase {
  public:
    Texture(const TextureDescriptor& descriptor);
    Texture(const TextureDescriptor& descriptor, MTL::Texture* mtl_texture_);
    ~Texture() override;

    TextureBase* CreateView(const TextureViewDescriptor& descriptor) override;

    // Copying
    void CopyFrom(const BufferBase* src, const usize src_stride,
                  const uint3 dst_origin, const usize3 size) override;
    void CopyFrom(const TextureBase* src, const uint3 src_origin,
                  const uint3 dst_origin, const usize3 size) override;

    // Blitting
    void BlitFrom(const TextureBase* src, const float3 src_origin,
                  const usize3 src_size, const float3 dst_origin,
                  const usize3 dst_size) override;

  private:
    bool owns_base{false};
    MTL::Texture* base_texture;
    MTL::Texture* texture;

    MTL::PixelFormat pixel_format;

    MTL::Texture* CreateViewImpl(TextureFormat format,
                                 SwizzleChannels swizzle_channels);

  public:
    GETTER(texture, GetTexture);
    GETTER(pixel_format, GetPixelFormat);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
