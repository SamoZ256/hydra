#pragma once

#include "common/io/continuous_stream.hpp"

namespace hydra::io {

class MemoryStream : public IContinuousStream {
  public:
    MemoryStream(std::span<u8> data_) : data{data_} {}

    MAKE_DEFAULT_COPYABLE(MemoryStream);
    MAKE_DEFAULT_MOVABLE(MemoryStream);

    u64 GetSize() const override { return data.size(); }

    u8* GetPtr() override { return data.data(); }

  private:
    std::span<u8> data;
};

} // namespace hydra::io
