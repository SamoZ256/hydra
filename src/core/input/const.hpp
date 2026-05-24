#pragma once

#include "core/horizon/services/hid/const.hpp"

namespace hydra::input {

enum class DeviceType : u32 {
    Invalid,

    Keyboard,
    Controller,
    Cursor,
};

struct Code {
    DeviceType device_type;
    u32 value;

    Code() : device_type{DeviceType::Invalid} {}
    template <typename T = u32>
    Code(DeviceType device_type_, T value_)
        : device_type{device_type_}, value{u32(value_)} {}

    GETTER(device_type, GetDeviceType);

    template <typename T = u32>
    T GetValue() const {
        return static_cast<T>(value);
    }
};

struct CodeButtonMapping {
    Code code;
    horizon::services::hid::NpadButtons npad_buttons;
};

enum class AnalogStickDirection {
    Right,
    Left,
    Up,
    Down,
};

struct AnalogStickAxis {
    bool is_left;
    AnalogStickDirection direction;
};

struct CodeAnalogMapping {
    Code code;
    AnalogStickAxis axis;
};

} // namespace hydra::input

ENABLE_ENUM_FORMATTING_AND_CASTING(hydra::input, DeviceType, Keyboard,
                                   "Keyboard", Controller, "Controller", Cursor,
                                   "Cursor")
