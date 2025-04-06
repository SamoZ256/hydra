#include "hw/tegra_x1/gpu/renderer/metal/texture.hpp"

#include "hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace Hydra::HW::TegraX1::GPU::Renderer::Metal {

Texture::Texture(const TextureDescriptor& descriptor)
    : TextureBase(descriptor) {
    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setWidth(descriptor.width);
    desc->setHeight(descriptor.height);

    pixel_format = to_mtl_pixel_format(descriptor.format);
    desc->setPixelFormat(pixel_format);

    mtl_texture = Renderer::GetInstance().GetDevice()->newTexture(desc);
}

Texture::Texture(const TextureDescriptor& descriptor,
                 MTL::Texture* mtl_texture_)
    : TextureBase(descriptor), mtl_texture{mtl_texture_} {}

Texture::~Texture() { mtl_texture->release(); }

TextureBase* Texture::CreateView(const TextureViewDescriptor& descriptor) {
    auto mtl_view =
        mtl_texture->newTextureView(to_mtl_pixel_format(descriptor.format));

    // TODO: don't pass this descriptor
    return new Texture(GetDescriptor(), mtl_view);
}

void Texture::CopyFrom(const void* data) {
    // TODO: do a GPU copy
    // TODO: bytes per image
    mtl_texture->replaceRegion(
        MTL::Region{0, 0, 0, mtl_texture->width(), mtl_texture->height(), 1}, 0,
        0, data, GetDescriptor().stride, 0);
}

void Texture::CopyFrom(const BufferBase* src, const usize src_stride,
                       const u32 dst_layer, const uint3 dst_origin,
                       const usize3 size) {
    const auto mtl_src = static_cast<const Buffer*>(src)->GetBuffer();

    auto encoder = Renderer::GetInstance().GetBlitCommandEncoder();

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

    auto encoder = Renderer::GetInstance().GetBlitCommandEncoder();

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
    Renderer::GetInstance().BlitTexture(
        static_cast<const Texture*>(src)->GetTexture(), src_origin, src_size,
        mtl_texture, dst_layer, dst_origin, dst_size);
}

} // namespace Hydra::HW::TegraX1::GPU::Renderer::Metal
