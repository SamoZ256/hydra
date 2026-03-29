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
    void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src,
                  const uint3 dst_origin, const usize3 size,
                  const Range<u32> levels, const Range<u32> layers) override;
    void CopyFrom(ICommandBuffer* command_buffer, const TextureBase* src,
                  const uint3 src_origin, const u32 src_level,
                  const u32 src_layer, const uint3 dst_origin,
                  const u32 dst_level, const u32 dst_layer, const usize3 size,
                  const u32 level_count, const u32 layer_count) override;

    // Blitting
    void BlitFrom(ICommandBuffer* command_buffer, const TextureBase* src,
                  const float3 src_origin, const usize3 src_size,
                  const u32 src_level, const u32 src_layer,
                  const float3 dst_origin, const usize3 dst_size,
                  const u32 dst_level, const u32 dst_layer,
                  const u32 level_count, const u32 layer_count) override;

  private:
    bool owns_base{false};
    MTL::Texture* base_texture;
    MTL::Texture* texture;

    MTL::PixelFormat pixel_format;

    MTL::Texture* CreateViewImpl(const TextureViewDescriptor& descriptor);

  public:
    GETTER(texture, GetTexture);
    GETTER(pixel_format, GetPixelFormat);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
