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
    void CopyFrom(const uptr data) override;
    void CopyFrom(const BufferBase* src, const usize src_stride,
                  const u32 dst_layer, const uint3 dst_origin,
                  const usize3 size) override;
    void CopyFrom(const TextureBase* src, const u32 src_layer,
                  const uint3 src_origin, const u32 dst_layer,
                  const uint3 dst_origin, const usize3 size) override;

    // Blitting
    void BlitFrom(const TextureBase* src, const u32 src_layer,
                  const float3 src_origin, const usize3 src_size,
                  const u32 dst_layer, const float3 dst_origin,
                  const usize3 dst_size) override;

    // Getters
    MTL::Texture* GetTexture() const { return mtl_texture; }

    MTL::PixelFormat GetPixelFormat() const { return pixel_format; }

  private:
    MTL::Texture* mtl_texture;

    MTL::PixelFormat pixel_format;
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
