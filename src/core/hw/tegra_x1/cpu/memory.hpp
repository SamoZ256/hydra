#pragma once

#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::hw::tegra_x1::cpu {

class IMemory {
  public:
    IMemory(u64 size_) : size{align(size_, GUEST_PAGE_SIZE)} {}
    virtual ~IMemory() = default;

    // The memory needs to be unmapped before resizing
    void Resize(u64 new_size) {
        size = new_size;
        ResizeImpl();
    }

    virtual uptr GetPtr() const = 0;

  protected:
    virtual void ResizeImpl() = 0;

  private:
    u64 size;

  public:
    GETTER(size, GetSize);
};

} // namespace hydra::hw::tegra_x1::cpu
