#pragma once

#include "core/hw/tegra_x1/gpu/renderer/buffer_base.hpp"

namespace hydra::hw::tegra_x1::gpu::renderer {

struct BufferView {
  public:
    BufferView() = default;
    BufferView(BufferBase* base_, u64 offset_ = 0, u64 size_ = invalid<u64>())
        : base{base_}, offset{offset_}, size{size_} {
        if (size == invalid<u64>())
            size = base->GetSize() - offset;
    }

    bool IsValid() const { return base != nullptr; }

    uptr GetPtr() const { return base->GetPtr() + offset; }

    // Copying
    void CopyFrom(const uptr data, u64 size_ = invalid<u64>()) {
        if (size_ == invalid<u64>())
            size_ = size - offset;
        base->CopyFrom(data, offset, size_);
    }
    void CopyFrom(ICommandBuffer* command_buffer, const BufferView& src,
                  u64 size_ = invalid<u64>()) {
        if (size_ == invalid<u64>())
            size_ = std::min(src.size - src.offset, size - offset);
        base->CopyFrom(command_buffer, src.base, offset, src.offset, size_);
    }
    void CopyFrom(ICommandBuffer* command_buffer, TextureBase* src,
                  const uint3 src_origin, const uint3 src_size) {
        base->CopyFrom(command_buffer, src, src_origin, src_size, offset);
    }

  protected:
    BufferBase* base{nullptr};
    u64 offset{0};
    u64 size{0};

  public:
    GETTER(base, GetBase);
    GETTER(offset, GetOffset);
    GETTER(size, GetSize);
};

} // namespace hydra::hw::tegra_x1::gpu::renderer
