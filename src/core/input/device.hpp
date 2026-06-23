#pragma once

#include "core/input/const.hpp"

namespace hydra::input {

class IDevice {
  public:
    virtual ~IDevice() = default;

    virtual bool ActsAsController() const { return false; };
    virtual bool ActsAsTouchScreen() const { return false; };

    // Controller
    virtual bool IsPressed([[maybe_unused]] const Code& code) { return false; }
    virtual f32 GetAxisValue([[maybe_unused]] const Code& code) { return 0.0f; }

    // Touch screen
    virtual u64 GetNextBeganTouchID() { return invalid<u64>(); };
    virtual u64 GetNextEndedTouchID() { return invalid<u64>(); };
    virtual void GetTouchPosition([[maybe_unused]] u64 id,
                                  [[maybe_unused]] i32& out_x,
                                  [[maybe_unused]] i32& out_y){};
};

} // namespace hydra::input
