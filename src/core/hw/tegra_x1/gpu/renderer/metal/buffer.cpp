#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"
#include "core/hw/tegra_x1/gpu/renderer/metal/texture.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Buffer::Buffer(u64 size) : BufferBase(size) {
    buffer = METAL_RENDERER_INSTANCE.GetDevice()->newBuffer(
        size, MTL::ResourceStorageModePrivate);
}

Buffer::Buffer(MTL::Buffer* buffer_)
    : BufferBase(buffer_->allocatedSize()), buffer{buffer_} {}

Buffer::~Buffer() { buffer->release(); }

void Buffer::CopyFrom(TextureBase* src, const uint3 src_origin,
                      const uint3 src_size, u64 dst_offset) {
    auto src_impl = static_cast<Texture*>(src);

    auto blit_encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();
    // TODO: bytes per image
    // TODO: calculate the stride for the Metal pixel format
    blit_encoder->copyFromTexture(
        src_impl->GetTexture(), 0, 0,
        MTL::Origin::Make(src_origin.x(), src_origin.y(), src_origin.z()),
        MTL::Size::Make(src_size.x(), src_size.y(), src_size.z()), buffer,
        dst_offset,
        get_texture_format_stride(src_impl->GetDescriptor().format,
                                  src_size.x()),
        0);
}

void Buffer::CopyFromImpl(const uptr data, u64 dst_offset, u64 size_) {
    memcpy((u8*)buffer->contents() + dst_offset, reinterpret_cast<void*>(data),
           size_);
}

void Buffer::CopyFromImpl(BufferBase* src, u64 dst_offset, u64 src_offset,
                          u64 size_) {
    auto src_impl = static_cast<Buffer*>(src);

    auto blit_encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();
    blit_encoder->copyFromBuffer(src_impl->GetBuffer(), src_offset, buffer,
                                 dst_offset, size_);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
