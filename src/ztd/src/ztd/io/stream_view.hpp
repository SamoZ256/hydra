#pragma once

#include "ztd/io/stream.hpp"

namespace ztd::io {

class StreamView : public IStream {
  public:
    StreamView(IStream* base_, u64 offset_, u64 size_) noexcept
        : base{base_}, offset{offset_}, size{size_} {
        base->seekTo(offset);
    }
    StreamView(IStream* base_, u64 offset_) noexcept
        : StreamView(base_, offset_, base_->getSize() - offset_) {}

    [[nodiscard]] auto getSeek() const noexcept -> u64 override {
        return base->getSeek() - offset;
    }
    auto seekTo(u64 seek) noexcept -> void override {
        base->seekTo(offset + seek);
    }
    auto seekBy(u64 offset_) noexcept -> void override {
        base->seekBy(offset_);
    }

    [[nodiscard]] auto getSize() const noexcept -> u64 override { return size; }

    auto flush() noexcept -> void override { base->flush(); }

    auto readRaw(std::span<u8> buffer) noexcept -> void override {
        base->readRaw(buffer);
    }
    auto writeRaw(std::span<const u8> buffer) noexcept -> void override {
        base->writeRaw(buffer);
    }
    [[nodiscard]] auto consumePtrRaw(usize size_) noexcept -> u8* override {
        return base->consumePtrRaw(size_);
    }

  private:
    IStream* base;
    u64 offset;
    u64 size;
};

} // namespace ztd::io
