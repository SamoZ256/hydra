#pragma once

#include "core/hw/tegra_x1/gpu/renderer/metal/const.hpp"
#include "core/hw/tegra_x1/gpu/renderer/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

class Texture final : public ITexture {
  public:
    Texture(const TextureDescriptor& descriptor);
    ~Texture() override;

    ITextureView*
    CreateView(const TextureViewDescriptor& view_descriptor) override;

    // Copying
    void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src,
                  const uint3 dst_origin, const usize3 dst_size,
                  const Range<u32> dst_levels,
                  const Range<u32> dst_layers) override;
    void CopyFrom(ICommandBuffer* command_buffer, const ITexture* src,
                  const uint3 src_origin, const u32 src_level,
                  const u32 src_layer, const uint3 dst_origin,
                  const u32 dst_level, const u32 dst_layer, const usize3 size,
                  const u32 level_count, const u32 layer_count) override;

    // Blitting
    void BlitFrom(ICommandBuffer* command_buffer, const ITexture* src,
                  const float3 src_origin, const usize3 src_size,
                  const u32 src_level, const u32 src_layer,
                  const float3 dst_origin, const usize3 dst_size,
                  const u32 dst_level, const u32 dst_layer,
                  const u32 level_count, const u32 layer_count) override;

  private:
    MTL::Texture* texture;

  public:
    GETTER(texture, GetTexture);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
