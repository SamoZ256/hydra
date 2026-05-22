#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/command_buffer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/maxwell_to_mtl.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture_view.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Texture::Texture(MTL::Device* device, const TextureDescriptor& descriptor)
    : ITexture(descriptor) {
    const auto type = ToMtlTextureType(descriptor.type);

    MTL::TextureDescriptor* desc = MTL::TextureDescriptor::alloc()->init();
    desc->setTextureType(type);
    desc->setWidth(descriptor.width);
    desc->setHeight(descriptor.height);
    desc->setDepth(descriptor.depth);
    desc->setMipmapLevelCount(descriptor.level_count);
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

    texture = device->newTexture(desc);
}

Texture::~Texture() { texture->release(); }

ITextureView*
Texture::CreateView(const TextureViewDescriptor& view_descriptor) {
    return new TextureView(this, view_descriptor);
}

void Texture::CopyFrom(ICommandBuffer* command_buffer, const BufferBase* src,
                       const Range<u32> dst_levels,
                       const Range<u32> dst_layers) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    const auto mtl_src = static_cast<const Buffer*>(src)->GetBuffer();

    auto encoder = command_buffer_impl->GetBlitCommandEncoder();

    u32 offset = 0;
    for (u32 layer = dst_layers.GetBegin(); layer < dst_layers.GetEnd();
         layer++) {
        for (u32 level = dst_levels.GetBegin(); level < dst_levels.GetEnd();
             level++) {
            // Calculate sizes
            const auto dims = descriptor.GetLevelDimensions(level);
            const auto stride =
                GetTextureFormatStride(descriptor.format, dims.x());
            const auto slice_stride = GetTextureFormatSliceStride(
                descriptor.format, dims.x(), dims.y());

            // Copy
            encoder->copyFromBuffer(mtl_src, offset, stride, slice_stride,
                                    MTL::Size(dims.x(), dims.y(), dims.z()),
                                    texture, layer, level,
                                    MTL::Origin(0, 0, 0));

            // Add offset
            offset += dims.z() * slice_stride;
        }
    }
}

// TODO: make sure source and destination sizes match
void Texture::CopyFrom(ICommandBuffer* command_buffer, const ITexture* src,
                       const u32 src_level, const u32 src_layer,
                       const u32 dst_level, const u32 dst_layer,
                       const u32 level_count, const u32 layer_count) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    const auto mtl_src = static_cast<const Texture*>(src)->GetTexture();

    auto encoder = command_buffer_impl->GetBlitCommandEncoder();

    for (u32 i = 0; i < layer_count; i++) {
        for (u32 j = 0; j < level_count; j++) {
            const u32 crnt_dst_level = dst_level + j;
            const auto dims = descriptor.GetLevelDimensions(crnt_dst_level);
            encoder->copyFromTexture(
                mtl_src, src_layer + i, src_level + j, MTL::Origin(0, 0, 0),
                MTL::Size(dims.x(), dims.y(), dims.z()), texture, dst_layer + i,
                crnt_dst_level, MTL::Origin(0, 0, 0));
        }
    }
}

void Texture::CopyFrom(ICommandBuffer* command_buffer, const ITexture* src,
                       const uint3 src_origin, const u32 src_level,
                       const u32 src_layer, const uint3 dst_origin,
                       const u32 dst_level, const u32 dst_layer,
                       const uint3 size, const u32 layer_count) {
    const auto command_buffer_impl =
        static_cast<CommandBuffer*>(command_buffer);
    const auto mtl_src = static_cast<const Texture*>(src)->GetTexture();

    auto encoder = command_buffer_impl->GetBlitCommandEncoder();

    for (u32 i = 0; i < layer_count; i++) {
        encoder->copyFromTexture(
            mtl_src, src_layer + i, src_level,
            MTL::Origin(src_origin.x(), src_origin.y(), src_origin.z()),
            MTL::Size(size.x(), size.y(), size.z()), texture, dst_layer + i,
            dst_level,
            MTL::Origin(dst_origin.x(), dst_origin.y(), dst_origin.z()));
    }
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
