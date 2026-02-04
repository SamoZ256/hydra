#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class TextureBase;

class BufferBase {
  public:
    BufferBase(u64 size_) : size{size_} {}
    virtual ~BufferBase() = default;

    virtual uptr GetPtr() const = 0;

    // Copying
    void CopyFrom(const uptr data, u64 dst_offset = 0,
                  u64 size_ = invalid<u64>()) {
        if (size_ == invalid<u64>())
            size_ = size - dst_offset;
        CopyFromImpl(data, dst_offset, size_);
    }
    void CopyFrom(ICommandBuffer* command_buffer, BufferBase* src,
                  u64 dst_offset = 0, u64 src_offset = 0,
                  u64 size_ = invalid<u64>()) {
        if (size_ == invalid<u64>())
            size_ = std::min(src->GetSize() - src_offset, size - dst_offset);
        CopyFromImpl(command_buffer, src, dst_offset, src_offset, size_);
    }
    virtual void CopyFrom(ICommandBuffer* command_buffer, TextureBase* src,
                          const uint3 src_origin, const uint3 src_size,
                          u64 dst_offset = 0) = 0;

  protected:
    u64 size;

    // Copying
    virtual void CopyFromImpl(const uptr data, u64 dst_offset, u64 size) = 0;
    virtual void CopyFromImpl(ICommandBuffer* command_buffer, BufferBase* src,
                              u64 dst_offset, u64 src_offset, u64 size) = 0;

  public:
    GETTER(size, GetSize);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
