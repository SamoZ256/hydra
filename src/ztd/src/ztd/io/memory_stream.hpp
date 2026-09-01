#pragma once

#include "ztd/io/continuous_stream.hpp"

namespace ztd::io {

class MemoryStream : public IContinuousStream {
  public:
    MemoryStream(std::span<u8> data_) noexcept : data{data_} {}
    ~MemoryStream() noexcept override = default;

    ZTD_MAKE_DEFAULT_COPYABLE(MemoryStream);
    ZTD_MAKE_DEFAULT_MOVABLE(MemoryStream);

    [[nodiscard]] auto getSize() const noexcept -> u64 override {
        return data.size();
    }
    [[nodiscard]] auto getPtr() noexcept -> u8* override { return data.data(); }

  private:
    std::span<u8> data;
};

} // namespace ztd::io
