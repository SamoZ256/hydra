#pragma once

#include "core/hw/tegra_x1/gpu/renderer/const.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

class ICommandBuffer;
class ITexture;

class BufferBase {
  public:
    explicit BufferBase(u64 size_) : size{size_} {}
    virtual ~BufferBase() = default;

    virtual uptr getPtr() const = 0;

    // Copying
    void copyFrom(const uptr data, u64 dst_offset = 0,
                  u64 size_ = invalid<u64>()) {
        if (size_ == invalid<u64>())
            size_ = size - dst_offset;
        copyFromImpl(data, dst_offset, size_);
    }
    void copyFrom(ICommandBuffer* command_buffer, BufferBase* src,
                  u64 dst_offset = 0, u64 src_offset = 0,
                  u64 size_ = invalid<u64>()) {
        if (size_ == invalid<u64>())
            size_ = std::min(src->getSize() - src_offset, size - dst_offset);
        copyFromImpl(command_buffer, src, dst_offset, src_offset, size_);
    }
    virtual void copyFrom(ICommandBuffer* command_buffer, ITextureView* src,
                          const uint3 src_origin, const uint3 src_size,
                          const ztd::Range<u32> src_levels,
                          const ztd::Range<u32> src_layers,
                          u64 dst_offset = 0) = 0;

  protected:
    u64 size;

    // Copying
    virtual void copyFromImpl(const uptr data, u64 dst_offset, u64 size) = 0;
    virtual void copyFromImpl(ICommandBuffer* command_buffer, BufferBase* src,
                              u64 dst_offset, u64 src_offset, u64 size) = 0;

  public:
    GETTER(size, getSize);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
