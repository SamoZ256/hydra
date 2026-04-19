#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class BufferBase;
class ITextureView;

class ITexture {
  public:
    ITexture(const TextureDescriptor& descriptor_) : descriptor{descriptor_} {}
    virtual ~ITexture() = default;

    virtual ITextureView*
    CreateView(const TextureViewDescriptor& view_descriptor) = 0;

    // Copying
    virtual void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src,
                          const uint3 dst_origin, const usize3 dst_size,
                          const Range<u32> dst_levels,
                          const Range<u32> dst_layers) = 0;
    void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src) {
        CopyFrom(
            command_buffer, src, uint3({0, 0, 0}),
            usize3({descriptor.width, descriptor.height, descriptor.depth}),
            Range<u32>(0, descriptor.level_count),
            Range<u32>(0, descriptor.layer_count));
    }
    virtual void CopyFrom(ICommandBuffer* command_buffer, const ITexture* src,
                          const uint3 src_origin, const u32 src_level,
                          const u32 src_layer, const uint3 dst_origin,
                          const u32 dst_level, const u32 dst_layer,
                          const usize3 size, const u32 level_count,
                          const u32 layer_count) = 0;

    // Blitting
    virtual void BlitFrom(ICommandBuffer* command_buffer, const ITexture* src,
                          const float3 src_origin, const usize3 src_size,
                          const u32 src_level, const u32 src_layer,
                          const float3 dst_origin, const usize3 dst_size,
                          const u32 dst_level, const u32 dst_layer,
                          const u32 level_count, const u32 layer_count) = 0;

  protected:
    const TextureDescriptor descriptor;

  public:
    CONST_REF_GETTER(descriptor, GetDescriptor);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
