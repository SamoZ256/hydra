#pragma once

#include "core/input/device.hpp"

namespace hydra::input {

enum class Key {
    Q,
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

class IKeyboard : public IDevice {
  public:
    bool ActsAsController() const override { return true; }

    // Controller
    bool IsPressed(const Code& code) override {
        if (code.GetDeviceType() != DeviceType::Keyboard)
            return false;

        const auto key = code.GetValue<Key>();
        return IsPressedImpl(key);
    }

    f32 GetAxisValue(const Code& code) override {
        if (code.GetDeviceType() != DeviceType::Keyboard)
            return 0.0f;

        const auto key = code.GetValue<Key>();
        return IsPressedImpl(key) ? 1.0f : 0.0f;
    }

  protected:
    virtual bool IsPressedImpl(Key key) = 0;
};

} // namespace hydra::input

ENABLE_ENUM_FORMATTING_AND_CASTING(
    hydra::input, Key, Q, "Q", W, "W", E, "E", R, "R", T, "T", Y, "Y", U, "U",
    I, "I", O, "O", P, "P", A, "A", S, "S", D, "D", F, "F", G, "G", H, "H", J,
    "J", K, "K", L, "L", Z, "Z", X, "X", C, "C", V, "V", B, "B", N, "N", M, "M",
    ArrowLeft, "Left", ArrowRight, "Right", ArrowUp, "Up", ArrowDown, "Down",
    Enter, "Enter", Tab, "Tab", Backspace, "Backspace", Space, "Space",
    LeftShift, "Left shift", RightShift, "Right shift", LeftControl,
    "Left control", RightControl, "Right control", LeftAlt, "Left alt",
    RightAlt, "Right alt", LeftSuper, "Left super", RightSuper, "Right super")
