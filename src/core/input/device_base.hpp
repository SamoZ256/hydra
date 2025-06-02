#pragma once

#include "core/input/const.hpp"

namespace hydra::input {

class DeviceBase {
  public:
    virtual ~DeviceBase() {}

    virtual bool ActsAsController() const { return false; };
    virtual bool ActsAsTouchScreen() const { return false; };

    // Controller
    virtual bool IsPressed(code_t code) { return false; }
    virtual f32 GetAxisValue(code_t code) { return 0.0f; }

    // Touch screen
    virtual u64 GetNextBeganTouchID() { return invalid<u64>(); };
    virtual u64 GetNextEndedTouchID() { return invalid<u64>(); };
    virtual void GetTouchPosition(u64 id, i32& out_x, i32& out_y){};
};

} // namespace hydra::input
