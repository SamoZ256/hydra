#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class BufferBase;
class ITexture;

class ITextureView {
  public:
    ITextureView(ITexture* base_, const TextureViewDescriptor& descriptor_)
        : base{base_}, descriptor{descriptor_} {}
    virtual ~ITextureView() = default;

    // Copying
    void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src,
                  const uint3 dst_origin, const usize3 dst_size,
                  const Range<u32> dst_levels, const Range<u32> dst_layers);
    void CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src);
    void CopyFrom(ICommandBuffer* command_buffer, const ITextureView* src,
                  const uint3 src_origin, const u32 src_level,
                  const u32 src_layer, const uint3 dst_origin,
                  const u32 dst_level, const u32 dst_layer, const usize3 size,
                  const u32 level_count, const u32 layer_count);

    // Blitting
    void BlitFrom(ICommandBuffer* command_buffer, const ITextureView* src,
                  const float3 src_origin, const usize3 src_size,
                  const u32 src_level, const u32 src_layer,
                  const float3 dst_origin, const usize3 dst_size,
                  const u32 dst_level, const u32 dst_layer,
                  const u32 level_count, const u32 layer_count);

  protected:
    ITexture* base;
    const TextureViewDescriptor descriptor;

  public:
    GETTER(base, GetBase);
    CONST_REF_GETTER(descriptor, GetDescriptor);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
