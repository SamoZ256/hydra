#pragma once

#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::hw::tegra_x1::cpu {

class IMemory {
  public:
    IMemory(usize size_) : size{size_} {}
    virtual ~IMemory() = default;

    virtual uptr GetPtr() const = 0;

  private:
    usize size;

  public:
    GETTER(size, GetSize);
};

} // namespace hydra::hw::tegra_x1::cpu
