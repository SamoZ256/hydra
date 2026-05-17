#include "core/input/sdl/controller.hpp"

#include <algorithm>

#include "frontend/sdl3/const.hpp"

namespace hydra::input::sdl {

bool Controller::IsPressedImpl(ControllerInput input) {
#define BUTTON_CASE(input, sdl_button)                                         \
    case ControllerInput::input:                                               \
        return SDL_GetGamepadButton(handle, sdl_button);                       \
        break;

    switch (input) {
        BUTTON_CASE(A, SDL_GAMEPAD_BUTTON_EAST)
        BUTTON_CASE(B, SDL_GAMEPAD_BUTTON_SOUTH)
        BUTTON_CASE(X, SDL_GAMEPAD_BUTTON_NORTH)
        BUTTON_CASE(Y, SDL_GAMEPAD_BUTTON_WEST)
        BUTTON_CASE(StickL, SDL_GAMEPAD_BUTTON_LEFT_STICK)
        BUTTON_CASE(StickR, SDL_GAMEPAD_BUTTON_RIGHT_STICK)
        BUTTON_CASE(L, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)
        BUTTON_CASE(R, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)
    case ControllerInput::ZL:
        return SDL_GetGamepadAxis(handle, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
    case ControllerInput::ZR:
        return SDL_GetGamepadAxis(handle, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
        BUTTON_CASE(Plus, SDL_GAMEPAD_BUTTON_START);
        BUTTON_CASE(Minus, SDL_GAMEPAD_BUTTON_BACK);
        BUTTON_CASE(Left, SDL_GAMEPAD_BUTTON_DPAD_LEFT);
        BUTTON_CASE(Up, SDL_GAMEPAD_BUTTON_DPAD_UP);
        BUTTON_CASE(Right, SDL_GAMEPAD_BUTTON_DPAD_RIGHT);
        BUTTON_CASE(Down, SDL_GAMEPAD_BUTTON_DPAD_DOWN);
    // TODO: implement
    case ControllerInput::LeftSL:
    case ControllerInput::LeftSR:
    case ControllerInput::RightSL:
    case ControllerInput::RightSR:
        return false;
    default:
        LOG_NOT_IMPLEMENTED(Input, "Controller button {}", input);
        return false;
    }
}

f32 positive_axis_to_float(i16 value) {
    return std::max(static_cast<i16>(0), value) /
           static_cast<f32>(std::numeric_limits<i16>::max());
}

f32 negative_axis_to_float(i16 value) {
    return std::min(static_cast<i16>(0), value) /
           static_cast<f32>(std::numeric_limits<i16>::min());
}

f32 Controller::GetAxisValueImpl(ControllerInput input) {
#define AXIS_CASE(input, sdl_dpad, direction)                                  \
    case ControllerInput::input:                                               \
        value = SDL_GetGamepadAxis(handle, sdl_dpad);                          \
        dir = AnalogStickDirection::direction;                                 \
        break;

    AnalogStickDirection dir;
    i16 value;

    switch (input) {
        AXIS_CASE(StickLLeft, SDL_GAMEPAD_AXIS_LEFTX, Left)
        AXIS_CASE(StickLUp, SDL_GAMEPAD_AXIS_LEFTY, Up)
        AXIS_CASE(StickLRight, SDL_GAMEPAD_AXIS_LEFTX, Right)
        AXIS_CASE(StickLDown, SDL_GAMEPAD_AXIS_LEFTY, Down)
        AXIS_CASE(StickRLeft, SDL_GAMEPAD_AXIS_RIGHTX, Left)
        AXIS_CASE(StickRUp, SDL_GAMEPAD_AXIS_RIGHTY, Up)
        AXIS_CASE(StickRRight, SDL_GAMEPAD_AXIS_RIGHTX, Right)
        AXIS_CASE(StickRDown, SDL_GAMEPAD_AXIS_RIGHTY, Down)
    default:
        LOG_NOT_IMPLEMENTED(Input, "Controller axis {}", input);
        return 0.0f;
    }

    switch (dir) {
    case AnalogStickDirection::Right:
        return positive_axis_to_float(value);
    case AnalogStickDirection::Left:
        return negative_axis_to_float(value);
    case AnalogStickDirection::Up:
        return negative_axis_to_float(value);
    case AnalogStickDirection::Down:
        return positive_axis_to_float(value);
    }
}

} // namespace hydra::input::sdl
