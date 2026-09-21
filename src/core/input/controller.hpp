#pragma once

#include "core/input/device.hpp"

namespace hydra::input {

// Uses Nintendo Switch layout
enum class ControllerInput {
    A,
    B,
    X,
    Y,
    StickL,
    StickR,
    L,
    R,
    ZL,
    ZR,
    Plus,
    Minus,
    Left,
    Up,
    Right,
    Down,
    StickLLeft,
    StickLUp,
    StickLRight,
    StickLDown,
    StickRLeft,
    StickRUp,
    StickRRight,
    StickRDown,
    LeftSL,
    LeftSR,
    RightSL,
    RightSR,
};

inline bool controllerInputIsStick(ControllerInput input) {
    return input == ControllerInput::StickLLeft ||
           input == ControllerInput::StickLUp ||
           input == ControllerInput::StickLRight ||
           input == ControllerInput::StickLDown ||
           input == ControllerInput::StickRLeft ||
           input == ControllerInput::StickRUp ||
           input == ControllerInput::StickRRight ||
           input == ControllerInput::StickRDown;
}

class IController : public IDevice {
  public:
    bool actsAsController() const override { return true; }

    // Controller
    bool isPressed(const Code& code) override {
        if (code.getDeviceType() != DeviceType::Controller)
            return false;

        const auto input = code.getValue<ControllerInput>();
        if (controllerInputIsStick(input))
            return getAxisValueImpl(input) > 0.5f;
        else
            return isPressedImpl(input);
    }

    f32 getAxisValue(const Code& code) override {
        if (code.getDeviceType() != DeviceType::Controller)
            return 0.0f;

        const auto input = code.getValue<ControllerInput>();
        if (controllerInputIsStick(input))
            return getAxisValueImpl(input);
        else
            return isPressedImpl(input) ? 1.0f : 0.0f;
    }

  protected:
    virtual bool isPressedImpl(ControllerInput input) = 0;
    virtual f32 getAxisValueImpl(ControllerInput input) = 0;
};

} // namespace hydra::input

ENABLE_ENUM_FORMATTING_AND_CASTING(
    hydra::input, ControllerInput, A, "A", B, "B", X, "X", Y, "Y", StickL,
    "stick L", StickR, "stick R", L, "L", R, "R", ZL, "ZL", ZR, "ZR", Plus, "+",
    Minus, "-", Left, "left", Up, "up", Right, "right", Down, "down",
    StickLLeft, "stick L left", StickLUp, "stick L up", StickLRight,
    "stick L right", StickLDown, "stick L down", StickRLeft, "stick R left",
    StickRUp, "stick R up", StickRRight, "stick R right", StickRDown,
    "stick R down", LeftSL, "left SL", LeftSR, "left SR", RightSL, "right SL",
    RightSR, "right SR")
