#include "core/hw/tegra_x1/gpu/renderer/metal/buffer.hpp"

#include "core/hw/tegra_x1/gpu/renderer/metal/renderer.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer::metal {

Buffer::Buffer(const BufferDescriptor& descriptor) : BufferBase(descriptor) {
    buffer = METAL_RENDERER_INSTANCE.GetDevice()->newBuffer(
        reinterpret_cast<void*>(descriptor.ptr), descriptor.size,
        MTL::ResourceStorageModeShared, nullptr);
}

Buffer::Buffer(MTL::Buffer* buffer_, u32 offset_)
    : BufferBase({reinterpret_cast<uptr>(buffer_->contents()) + offset_,
                  buffer_->allocatedSize()}),
      buffer{buffer_}, offset{offset_} {
    owns_buffer = false;
}

Buffer::~Buffer() {
    if (owns_buffer)
        buffer->release();
}

void Buffer::CopyFrom(const uptr data) {
    // TODO: get the buffer from a buffer allocator instead
    auto tmp_buffer = METAL_RENDERER_INSTANCE.GetDevice()->newBuffer(
        descriptor.size, MTL::ResourceStorageModeShared);
    memcpy(tmp_buffer->contents(), reinterpret_cast<void*>(data),
           descriptor.size);

    auto blit_encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();
    blit_encoder->copyFromBuffer(tmp_buffer, 0, buffer, 0, descriptor.size);

    tmp_buffer->release();
}

void Buffer::CopyFrom(BufferBase* src) {
    auto src_impl = static_cast<Buffer*>(src);

    auto blit_encoder = METAL_RENDERER_INSTANCE.GetBlitCommandEncoder();
    blit_encoder->copyFromBuffer(src_impl->GetBuffer(), 0, buffer, 0,
                                 descriptor.size);
}

} // namespace hydra::hw::tegra_x1::gpu::renderer::metal
