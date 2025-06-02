#pragma once

#include "core/input/device_base.hpp"

namespace hydra::input {

enum class Key {
    Q = 0,
    W,
    E,
    R,
    T,
    Y,
    U,
    I,
    O,
    P,
    A,
    S,
    D,
    F,
    G,
    H,
    J,
    K,
    L,
    Z,
    X,
    C,
    V,
    B,
    N,
    M,

    // TODO: numbers

    // TODO: other stuff

    ArrowLeft,
    ArrowRight,
    ArrowUp,
    ArrowDown,

    Enter,
    Tab,
    Backspace,
    Space,

    LeftShift,
    RightShift,
    LeftControl,
    RightControl,
    LeftAlt,
    RightAlt,
    LeftSuper,
    RightSuper,
};

constexpr u32 KEYBOARD_DEVICE_ID = 1;

class KeyboardBase : public DeviceBase {
  public:
    bool ActsAsController() const override { return true; }

    // Controller
    bool IsPressed(code_t code) override {
        if (get_code_device_id(code) != KEYBOARD_DEVICE_ID)
            return false;

        const auto key = get_code_value<Key>(code);
        return IsPressedImpl(key);
    }

    f32 GetAxisValue(code_t code) override {
        if (get_code_device_id(code) != KEYBOARD_DEVICE_ID)
            return 0.0f;

        const auto key = get_code_value<Key>(code);
        return IsPressedImpl(key) ? 1.0f : 0.0f;
    }

  protected:
    virtual bool IsPressedImpl(Key key) = 0;
};

} // namespace hydra::input
