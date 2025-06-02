#pragma once

#include "core/input/device_base.hpp"

namespace hydra::input {

constexpr u32 CURSOR_DEVICE_ID = 2;

class CursorBase : public DeviceBase {
  public:
    bool ActsAsTouchScreen() const override { return true; }
};

} // namespace hydra::input
