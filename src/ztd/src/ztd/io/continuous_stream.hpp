#pragma once

#include <algorithm>

#include "ztd/io/stream.hpp"

namespace ztd::io {

class IContinuousStream : public IStream {
  public:
    IContinuousStream() noexcept = default;
    ~IContinuousStream() noexcept override = default;

    ZTD_MAKE_DEFAULT_COPYABLE(IContinuousStream);
    ZTD_MAKE_DEFAULT_MOVABLE(IContinuousStream);

    [[nodiscard]] auto getSeek() const noexcept -> u64 override { return seek; }
    auto seekTo(u64 seek_) noexcept -> void override { seek = seek_; }
    auto seekBy(u64 offset) noexcept -> void override { seek += offset; }

  protected:
    auto readRaw(std::span<u8> buffer) noexcept -> void override {
        auto* const ptr = consumePtrRaw(buffer.size());
        std::copy(ptr, ptr + buffer.size(), buffer.data());
    }

    auto writeRaw(std::span<const u8> buffer) noexcept -> void override {
        auto* ptr = consumePtrRaw(buffer.size());
        std::copy(buffer.data(), buffer.data() + buffer.size(), ptr);
    }

    [[nodiscard]] auto consumePtrRaw(usize size) noexcept -> u8* override {
        auto* const ptr = getPtr() + seek;
        seekBy(size);
        return ptr;
    }

    [[nodiscard]] virtual auto getPtr() noexcept -> u8* = 0;

  private:
    u64 seek{0};
};

} // namespace ztd::io
