#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Texture::Texture(const TextureDescriptor& descriptor)
    : TextureBase(descriptor) {
    const auto type = ToMtlTextureType(descriptor.type);

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setTextureType(type);
    desc->setWidth(descriptor.width);
    desc->setHeight(descriptor.height);
    desc->setStorageMode(MTL::StorageModePrivate);

    switch (descriptor.type) {
    case TextureType::_1DArray:
    case TextureType::_2DArray:
        desc->setArrayLength(descriptor.depth);
        break;
    case TextureType::CubeArray:
        // TODO: correct?
        ASSERT_DEBUG(descriptor.depth % 6 == 0, MetalRenderer,
                     "Invalid cube array depth {}", descriptor.depth);
        desc->setArrayLength(descriptor.depth / 6);
        break;
    case TextureType::_3D:
        desc->setDepth(descriptor.depth);
        break;
    default:
        ASSERT_DEBUG(descriptor.depth == 1, MetalRenderer,
                     "Invalid depth {} for type {}", descriptor.depth,
                     descriptor.type);
        break;
    }

    const auto& pixel_format_info = to_mtl_pixel_format_info(descriptor.format);
    pixel_format = pixel_format_info.pixel_format;
    desc->setPixelFormat(pixel_format);

    base_texture = METAL_RENDERER_INSTANCE.GetDevice()->newTexture(desc);
    if (pixel_format_info.component_indices == uchar4{0, 1, 2, 3}) {
        texture = base_texture;
    } else {
        owns_base = true;
        texture = CreateViewImpl(descriptor.format, SwizzleChannels());
    }
}

Texture::Texture(const TextureDescriptor& descriptor,
                 MTL::Texture* mtl_texture_)
    : TextureBase(descriptor), owns_base{false}, base_texture{mtl_texture_},
      texture{mtl_texture_} {}

Texture::~Texture() {
    if (owns_base)
        base_texture->release();
    texture->release();
}

TextureBase* Texture::CreateView(const TextureViewDescriptor& descriptor) {
    const auto& pixel_format_info = to_mtl_pixel_format_info(descriptor.format);

    // Swizzle
    MTL::TextureSwizzle swizzle_components[] = {
        to_mtl_swizzle(descriptor.swizzle_channels.r),
        to_mtl_swizzle(descriptor.swizzle_channels.g),
        to_mtl_swizzle(descriptor.swizzle_channels.b),
        to_mtl_swizzle(descriptor.swizzle_channels.a)};
    MTL::TextureSwizzleChannels swizzle_channels(
        swizzle_components[pixel_format_info.component_indices[0]],
        swizzle_components[pixel_format_info.component_indices[1]],
        swizzle_components[pixel_format_info.component_indices[2]],
        swizzle_components[pixel_format_info.component_indices[3]]);

    auto desc = GetDescriptor();
    desc.format = descriptor.format;
    desc.swizzle_channels = descriptor.swizzle_channels;

    return new Texture(
        desc, CreateViewImpl(descriptor.format, descriptor.swizzle_channels));
}

void Texture::CopyFrom(const BufferBase* src, const usize src_stride,
                       uint3 dst_origin, usize3 size) {
    const auto mtl_src = static_cast<const Buffer*>(src)->GetBuffer();

    auto encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();

    u32 dst_layer = 0;
    u32 layer_count = 1;
    if (descriptor.type != TextureType::_3D) {
        dst_layer = dst_origin.z();
        dst_origin.z() = 0;
        layer_count = static_cast<u32>(size.z());
        size.z() = 1;
    }

    const auto bytes_per_image = descriptor.depth * src_stride;
    for (u32 i = 0; i < layer_count; i++) {
        const auto crnt_dst_layer = dst_layer + i;
        encoder->copyFromBuffer(
            mtl_src, crnt_dst_layer * bytes_per_image, src_stride,
            bytes_per_image, MTL::Size(size.x(), size.y(), size.z()), texture,
            crnt_dst_layer, 0,
            MTL::Origin(dst_origin.x(), dst_origin.y(), dst_origin.z()));
    }
}

void Texture::CopyFrom(const TextureBase* src, uint3 src_origin,
                       uint3 dst_origin, usize3 size) {
    const auto mtl_src = static_cast<const Texture*>(src)->GetTexture();

    auto encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();

    u32 src_layer = 0;
    u32 dst_layer = 0;
    u32 layer_count = 1;
    if (descriptor.type != TextureType::_3D) {
        dst_layer = dst_origin.z();
        dst_origin.z() = 0;
    }

    if (src->GetDescriptor().type != TextureType::_3D) {
        src_layer = src_origin.z();
        src_origin.z() = 0;
    }

    if (descriptor.type != TextureType::_3D ||
        src->GetDescriptor().type != TextureType::_3D) {
        layer_count = static_cast<u32>(size.z());
        size.z() = 1;
    }

    for (u32 i = 0; i < layer_count; i++) {
        encoder->copyFromTexture(
            mtl_src, src_layer + i, 0,
            MTL::Origin(src_origin.x(), src_origin.y(), src_origin.z()),
            MTL::Size(size.x(), size.y(), size.z()), texture, dst_layer + i, 0,
            MTL::Origin(dst_origin.x(), dst_origin.y(), dst_origin.z()));
    }
}

void Texture::BlitFrom(const TextureBase* src, const float3 src_origin,
                       const usize3 src_size, const float3 dst_origin,
                       const usize3 dst_size) {
    METAL_RENDERER_INSTANCE.BlitTexture(
        static_cast<const Texture*>(src)->GetTexture(), src_origin, src_size,
        texture, 0, dst_origin, dst_size);
}

MTL::Texture* Texture::CreateViewImpl(TextureFormat format,
                                      SwizzleChannels swizzle_channels) {
    const auto& pixel_format_info = to_mtl_pixel_format_info(format);

    // Swizzle
    MTL::TextureSwizzle swizzle_components[] = {
        to_mtl_swizzle(swizzle_channels.r), to_mtl_swizzle(swizzle_channels.g),
        to_mtl_swizzle(swizzle_channels.b), to_mtl_swizzle(swizzle_channels.a)};
    MTL::TextureSwizzleChannels swizzle_channels_mtl(
        swizzle_components[pixel_format_info.component_indices[0]],
        swizzle_components[pixel_format_info.component_indices[1]],
        swizzle_components[pixel_format_info.component_indices[2]],
        swizzle_components[pixel_format_info.component_indices[3]]);

    // TODO: ranges and levels

    u32 levels = 1;
    switch (descriptor.type) {
    case TextureType::_1DArray:
    case TextureType::_2DArray:
    case TextureType::CubeArray:
        levels = descriptor.depth;
        break;
    case TextureType::Cube:
        // TODO: assert that depth is 6
        levels = 6;
        break;
    case TextureType::_3D:
        // TODO: assert that depth matches
        break;
    default:
        break;
    }

    return base_texture->newTextureView(
        to_mtl_pixel_format(format), ToMtlTextureType(this->descriptor.type),
        NS::Range(0, 1), NS::Range(0, levels), swizzle_channels_mtl);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
