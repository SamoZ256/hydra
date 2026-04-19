#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Texture::Texture(const TextureDescriptor& descriptor) : ITexture(descriptor) {
    const auto type = ToMtlTextureType(descriptor.type);

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setTextureType(type);
    desc->setWidth(descriptor.width);
    desc->setHeight(descriptor.height);
    desc->setDepth(descriptor.depth);
    // TODO
    // desc->setMipmapLevelCount(descriptor.level_count);
    desc->setStorageMode(MTL::StorageModePrivate);

    switch (descriptor.type) {
    case TextureType::_1DArray:
    case TextureType::_2DArray:
        desc->setArrayLength(descriptor.layer_count);
        break;
    case TextureType::Cube:
        ASSERT_DEBUG(descriptor.layer_count == 6, MetalRenderer,
                     "Invalid cube layer count {}", descriptor.layer_count);
        break;
    case TextureType::CubeArray:
        ASSERT_DEBUG(descriptor.layer_count % 6 == 0, MetalRenderer,
                     "Invalid cube array layer count {}",
                     descriptor.layer_count);
        desc->setArrayLength(descriptor.layer_count / 6);
        break;
    default:
        ASSERT_DEBUG(descriptor.layer_count == 1, MetalRenderer,
                     "Invalid {} layer count {}", descriptor.type,
                     descriptor.layer_count);
        break;
    }

    const auto& pixel_format_info = to_mtl_pixel_format_info(descriptor.format);
    desc->setPixelFormat(pixel_format_info.pixel_format);

    texture = METAL_RENDERER_INSTANCE.GetDevice()->newTexture(desc);
}

Texture::~Texture() { texture->release(); }

ITextureView*
Texture::CreateView(const TextureViewDescriptor& view_descriptor) {
    return new TextureView(this, view_descriptor);
}

void Texture::CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src,
                       const uint3 dst_origin, const usize3 dst_size,
                       const Range<u32> dst_levels,
                       const Range<u32> dst_layers) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    const auto mtl_src = static_cast<const Buffer*>(src)->GetBuffer();

    auto encoder = command_buffer_impl->GetBlitCommandEncoder();

    // TODO: bytes per image
    // TODO: don't align
    const auto stride = align(
        get_texture_format_stride(descriptor.format, descriptor.width), 64u);
    for (u32 layer = dst_layers.GetBegin(); layer < dst_layers.GetEnd();
         layer++) {
        for (u32 level = dst_levels.GetBegin(); level < dst_levels.GetEnd();
             level++) {
            encoder->copyFromBuffer(
                mtl_src,
                layer * descriptor.layer_size +
                    /*descriptor.GetLevelOffset(level)*/ 0,
                stride, descriptor.height * stride,
                MTL::Size(dst_size.x(), dst_size.y(), dst_size.z()), texture,
                layer, level,
                MTL::Origin(dst_origin.x(), dst_origin.y(), dst_origin.z()));
        }
    }
}

void Texture::CopyFrom(ICommandBuffer* command_buffer, const ITexture* src,
                       const uint3 src_origin, const u32 src_level,
                       const u32 src_layer, const uint3 dst_origin,
                       const u32 dst_level, const u32 dst_layer,
                       const usize3 size, const u32 level_count,
                       const u32 layer_count) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    const auto mtl_src = static_cast<const Texture*>(src)->GetTexture();

    auto encoder = command_buffer_impl->GetBlitCommandEncoder();

    // TODO: levels
    (void)level_count;
    for (u32 i = 0; i < layer_count; i++) {
        for (u32 j = 0; j < /*level_count*/ 1; j++) {
            encoder->copyFromTexture(
                mtl_src, src_layer + i, src_level /* + j*/,
                MTL::Origin(src_origin.x(), src_origin.y(), src_origin.z()),
                MTL::Size(size.x(), size.y(), size.z()), texture, dst_layer + i,
                dst_level /* + j*/,
                MTL::Origin(dst_origin.x(), dst_origin.y(), dst_origin.z()));
        }
    }
}

void Texture::BlitFrom(ICommandBuffer* command_buffer, const ITexture* src,
                       const float3 src_origin, const usize3 src_size,
                       const u32 src_level, const u32 src_layer,
                       const float3 dst_origin, const usize3 dst_size,
                       const u32 dst_level, const u32 dst_layer,
                       const u32 level_count, const u32 layer_count) {
    // TODO: support a wider range of parameters
    ASSERT_DEBUG(src_level == 0, MetalRenderer, "Unsupported source level {}",
                 src_level);
    ASSERT_DEBUG(src_layer == 0, MetalRenderer, "Unsupported source layer {}",
                 src_layer);
    ASSERT_DEBUG(dst_level == 0, MetalRenderer,
                 "Unsupported destination level {}", dst_level);
    ASSERT_DEBUG(dst_layer == 0, MetalRenderer,
                 "Unsupported destination layer {}", dst_layer);
    ASSERT_DEBUG(level_count == 1, MetalRenderer, "Unsupported level_count {}",
                 level_count);
    ASSERT_DEBUG(layer_count == 1, MetalRenderer, "Unsupported layer_count {}",
                 layer_count);

    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    METAL_RENDERER_INSTANCE.BlitTexture(
        command_buffer_impl, static_cast<const Texture*>(src)->GetTexture(),
        src_origin, src_size, texture, 0, dst_origin, dst_size);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
