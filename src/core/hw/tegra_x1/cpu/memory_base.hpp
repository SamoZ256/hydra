#pragma once

#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::hw::tegra_x1::cpu {

class MemoryBase {
  public:
    MemoryBase(usize size_) : size{size_} {}

    // Getters
    usize GetSize() const { return size; }

  private:
    usize size;
};

} // namespace hydra::hw::tegra_x1::cpu
