#pragma once

#include "core/input/device.hpp"

namespace hydra::input {

class ICursor : public IDevice {
  public:
    bool actsAsTouchScreen() const override { return true; }
};

} // namespace hydra::input
