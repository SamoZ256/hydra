#pragma once

#include "core/input/const.hpp"

namespace hydra::input {

class IDevice {
  public:
    virtual ~IDevice() = default;

    virtual bool actsAsController() const { return false; };
    virtual bool actsAsTouchScreen() const { return false; };

    // Controller
    virtual bool isPressed([[maybe_unused]] const Code& code) { return false; }
    virtual f32 getAxisValue([[maybe_unused]] const Code& code) { return 0.0f; }

    // Touch screen
    virtual u64 getNextBeganTouchId() { return invalid<u64>(); };
    virtual u64 getNextEndedTouchId() { return invalid<u64>(); };
    virtual void getTouchPosition([[maybe_unused]] u64 id,
                                  [[maybe_unused]] i32& out_x,
                                  [[maybe_unused]] i32& out_y) {};
};

} // namespace hydra::input
