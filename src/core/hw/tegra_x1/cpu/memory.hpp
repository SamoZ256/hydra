#pragma once

#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::hw::tegra_x1::cpu {

class IMemory {
  public:
    explicit IMemory(u64 size_) : size{align(size_, GUEST_PAGE_SIZE)} {}
    virtual ~IMemory() = default;

    ZTD_MAKE_NON_COPYABLE(IMemory);
    ZTD_MAKE_NON_MOVABLE(IMemory);

    // The memory needs to be unmapped before resizing
    void resize(u64 new_size) {
        size = new_size;
        resizeImpl();
    }

    virtual uptr getPtr() const = 0;

  protected:
    virtual void resizeImpl() = 0;

  private:
    u64 size;

  public:
    GETTER(size, getSize);
};

} // namespace hydra::hw::tegra_x1::cpu
