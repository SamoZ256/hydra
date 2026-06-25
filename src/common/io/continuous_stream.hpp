#pragma once

#include "common/io/stream.hpp"

namespace hydra::io {

class IContinuousStream : public IStream {
  public:
    IContinuousStream() noexcept = default;
    ~IContinuousStream() noexcept = default;

    MAKE_DEFAULT_COPYABLE(IContinuousStream);
    MAKE_DEFAULT_MOVABLE(IContinuousStream);

    u64 GetSeek() const override { return seek; }
    void SeekTo(u64 seek_) override { seek = seek_; }
    void SeekBy(u64 offset) override { seek += offset; }

  protected:
    void ReadRaw(std::span<u8> buffer) override {
        const auto ptr = ConsumePtrRaw(buffer.size());
        std::copy(ptr, ptr + buffer.size(), buffer.data());
    }

    void WriteRaw(std::span<const u8> buffer) override {
        auto ptr = ConsumePtrRaw(buffer.size());
        std::copy(buffer.data(), buffer.data() + buffer.size(), ptr);
    }

    u8* ConsumePtrRaw(usize size) override {
        const auto ptr = GetPtr() + seek;
        SeekBy(size);
        return ptr;
    }

    virtual u8* GetPtr() = 0;

  private:
    u64 seek{0};
};

} // namespace hydra::io
