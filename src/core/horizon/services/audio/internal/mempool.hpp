#pragma once

#include "core/horizon/services/audio/const.hpp"

namespace hydra::horizon::services::audio::internal {

class MemPool {
  public:
    bool Map(std::span<u8> data_) {
        if (mapped)
            return false;

        mapped = true;
        data = data_;
        return true;
    }

    bool Unmap() {
        if (!mapped)
            return false;

        mapped = false;
        return true;
    }

  private:
    bool mapped{false};
    std::span<u8> data;
};

} // namespace hydra::horizon::services::audio::internal
