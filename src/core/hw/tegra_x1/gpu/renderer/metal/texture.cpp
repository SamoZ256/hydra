#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Texture::Texture(const TextureDescriptor& descriptor)
    : TextureBase(descriptor) {
    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    // TODO: type
    desc->setWidth(descriptor.width);
    desc->setHeight(descriptor.height);

    pixel_format = to_mtl_pixel_format(descriptor.format);
    desc->setPixelFormat(pixel_format);

    mtl_texture = METAL_RENDERER_INSTANCE.GetDevice()->newTexture(desc);
}

Texture::Texture(const TextureDescriptor& descriptor,
                 MTL::Texture* mtl_texture_)
    : TextureBase(descriptor), mtl_texture{mtl_texture_} {}

Texture::~Texture() { mtl_texture->release(); }

TextureBase* Texture::CreateView(const TextureViewDescriptor& descriptor) {
    MTL::TextureSwizzleChannels swizzle_channels(
        to_mtl_swizzle(descriptor.swizzle_channels.r),
        to_mtl_swizzle(descriptor.swizzle_channels.g),
        to_mtl_swizzle(descriptor.swizzle_channels.b),
        to_mtl_swizzle(descriptor.swizzle_channels.a));

    // TODO: don't hardcode type, ranges and levels
    auto mtl_view = mtl_texture->newTextureView(
        to_mtl_pixel_format(descriptor.format), MTL::TextureType2D,
        NS::Range(0, 1), NS::Range(0, 1), swizzle_channels);

    auto desc = GetDescriptor();
    desc.format = descriptor.format;
    desc.swizzle_channels = descriptor.swizzle_channels;

    return new Texture(desc, mtl_view);
}

void Texture::CopyFrom(const uptr data) {
    usize size = descriptor.stride * descriptor.height * 1;

    // TODO: get the buffer from a buffer allocator instead
    auto tmp_buffer = METAL_RENDERER_INSTANCE.GetDevice()->newBuffer(
        size, MTL::ResourceStorageModeShared);
    memcpy(tmp_buffer->contents(), reinterpret_cast<void*>(data), size);

    auto encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();

    // TODO: bytes per image
    encoder->copyFromBuffer(tmp_buffer, 0, descriptor.stride, 0,
                            MTL::Size(descriptor.width, descriptor.height, 1),
                            mtl_texture, 0, 0, MTL::Origin(0, 0, 0));

    tmp_buffer->release();
}

void Texture::CopyFrom(const BufferBase* src, const usize src_stride,
                       const u32 dst_layer, const uint3 dst_origin,
                       const usize3 size) {
    const auto mtl_src = static_cast<const Buffer*>(src)->GetBuffer();

    auto encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();

    // TODO: bytes per image
    encoder->copyFromBuffer(
        mtl_src, 0, src_stride, 0, MTL::Size(size.x(), size.y(), size.z()),
        mtl_texture, dst_layer, 0,
        MTL::Origin(dst_origin.x(), dst_origin.y(), dst_origin.z()));
}

void Texture::CopyFrom(const TextureBase* src, const u32 src_layer,
                       const uint3 src_origin, const u32 dst_layer,
                       const uint3 dst_origin, const usize3 size) {
    const auto mtl_src = static_cast<const Texture*>(src)->GetTexture();

    auto encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();

    // TODO: bytes per image
    encoder->copyFromTexture(
        mtl_src, src_layer, 0,
        MTL::Origin(src_origin.x(), src_origin.y(), src_origin.z()),
        MTL::Size(size.x(), size.y(), size.z()), mtl_texture, dst_layer, 0,
        MTL::Origin(dst_origin.x(), dst_origin.y(), dst_origin.z()));
}

void Texture::BlitFrom(const TextureBase* src, const u32 src_layer,
                       const float3 src_origin, const usize3 src_size,
                       const u32 dst_layer, const float3 dst_origin,
                       const usize3 dst_size) {
    // TODO: src layer
    METAL_RENDERER_INSTANCE.BlitTexture(
        static_cast<const Texture*>(src)->GetTexture(), src_origin, src_size,
        mtl_texture, dst_layer, dst_origin, dst_size);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
