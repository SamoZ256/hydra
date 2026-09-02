#include "core/horizon/services/hid/internal/npad.hpp"

#include "core/horizon/kernel/event.hpp"

namespace hydra::horizon::services::hid::internal {

namespace {

i32 NormalizedFloatToInt(f32 value) {
    if (value < 0.0f)
        return std::max(
            static_cast<i16>(-value * std::numeric_limits<i16>::min()),
            std::numeric_limits<i16>::min());
    else
        return std::min(
            static_cast<i16>(value * std::numeric_limits<i16>::max()),
            std::numeric_limits<i16>::max());
}

// TODO: deadzone
int2 AnalogStickToInt(float2 value) {
    const auto length2 = value.x() * value.x() + value.y() * value.y();

    // Normalize if length is greater than 1
    if (length2 > 1.0f) {
        const auto length = std::sqrt(length2);
        value.x() /= length;
        value.y() /= length;
    }

    return int2(
        {NormalizedFloatToInt(value.x()), NormalizedFloatToInt(value.y())});
}

} // namespace

// TODO: should the event be signalled?
Npad::Npad(NpadInternalState& state_)
    : state{state_}, style_set_update_event{
                         new kernel::Event(false, "Style set update event")} {}

Npad::~Npad() { delete style_set_update_event; }

void Npad::Setup(NpadStyleSet style_set) {
    if (style_set == state.style_set)
        return;

    // Event
    // TODO: correct?
    style_set_update_event->Signal();

    // Update style set
    state.style_set = style_set;
    if (style_set == NpadStyleSet::None)
        return;

    // Colors
    // TODO: make this configurable
    state.full_key_color.full_key.main = NpadColor::BodyGray;
    state.full_key_color.full_key.sub = NpadColor::ButtonGray;
    state.joy_color.left.main = NpadColor::BodyNeonBlue;
    state.joy_color.left.sub = NpadColor::ButtonGray;
    state.joy_color.right.main = NpadColor::BodyNeonRed;
    state.joy_color.right.sub = NpadColor::ButtonGray;

    state.system_properties = NpadSystemProperties::IsPoweredJoyDual |
                              NpadSystemProperties::IsPoweredJoyLeft |
                              NpadSystemProperties::IsPoweredJoyRight;

    // Battery
    state.battery_level_joy_dual = NpadBatteryLevel::Percent100;
    state.battery_level_joy_left = NpadBatteryLevel::Percent100;
    state.battery_level_joy_right = NpadBatteryLevel::Percent100;

    switch (style_set) {
    case NpadStyleSet::FullKey:
        state.device_type = DeviceTypeBits::FullKey;
        state.joy_assignment_mode = NpadJoyAssignmentMode::Dual;
        state.system_properties |= NpadSystemProperties::IsAbxyButtonOriented |
                                   NpadSystemProperties::IsPlusAvailable |
                                   NpadSystemProperties::IsMinusAvailable;
        state.applet_footer_ui_type = AppletFooterUiType::SwitchProController;
        break;
    case NpadStyleSet::Handheld:
        state.device_type =
            DeviceTypeBits::HandheldLeft | DeviceTypeBits::HandheldRight;
        state.joy_assignment_mode = NpadJoyAssignmentMode::Dual;
        state.system_properties |= NpadSystemProperties::IsAbxyButtonOriented |
                                   NpadSystemProperties::IsPlusAvailable |
                                   NpadSystemProperties::IsMinusAvailable;
        state.applet_footer_ui_type =
            AppletFooterUiType::HandheldJoyConLeftJoyConRight;
        break;
    case NpadStyleSet::JoyDual:
        state.device_type = DeviceTypeBits::JoyLeft | DeviceTypeBits::JoyRight;
        state.joy_assignment_mode = NpadJoyAssignmentMode::Dual;
        state.system_properties |= NpadSystemProperties::IsAbxyButtonOriented |
                                   NpadSystemProperties::IsPlusAvailable |
                                   NpadSystemProperties::IsMinusAvailable;
        state.applet_footer_ui_type =
            CONFIG_INSTANCE.GetHandheldMode()
                ? AppletFooterUiType::HandheldJoyConLeftJoyConRight
                : AppletFooterUiType::JoyDual;
        break;
    case NpadStyleSet::JoyLeft:
        state.device_type = DeviceTypeBits::JoyLeft;
        state.joy_assignment_mode = NpadJoyAssignmentMode::Single;
        state.system_properties |= NpadSystemProperties::IsSlSrButtonOriented |
                                   NpadSystemProperties::IsMinusAvailable;
        state.applet_footer_ui_type =
            CONFIG_INSTANCE.GetHandheldMode()
                ? AppletFooterUiType::HandheldJoyConLeftOnly
                : AppletFooterUiType::JoyDualLeftOnly;
        break;
    case NpadStyleSet::JoyRight:
        state.device_type = DeviceTypeBits::JoyRight;
        state.joy_assignment_mode = NpadJoyAssignmentMode::Single;
        state.system_properties |= NpadSystemProperties::IsSlSrButtonOriented |
                                   NpadSystemProperties::IsPlusAvailable;
        state.applet_footer_ui_type =
            CONFIG_INSTANCE.GetHandheldMode()
                ? AppletFooterUiType::HandheldJoyConRightOnly
                : AppletFooterUiType::JoyDualRightOnly;
        break;
    case NpadStyleSet::Palma:
        state.device_type = DeviceTypeBits::Palma;
        state.joy_assignment_mode = NpadJoyAssignmentMode::Dual;
        state.applet_footer_ui_type = AppletFooterUiType::None;
        break;
    default:
        LOG_FATAL(Services, "Unsupported style set {}", style_set);
    }
}

void Npad::Update(const input::NpadState& new_state) {
    if (state.style_set == NpadStyleSet::None)
        return;

    // State
    const auto analog_l = AnalogStickToInt(new_state.analog_l);
    const auto analog_r = AnalogStickToInt(new_state.analog_r);
    NpadCommonState new_state_entry{
        .buttons = new_state.buttons,
        .analog_stick_l =
            {
                .x = analog_l.x(),
                .y = analog_l.y(),
            },
        .analog_stick_r =
            {
                .x = analog_r.x(),
                .y = analog_r.y(),
            },
        .attributes = NpadAttributes::IsConnected,
    };

    // Attributes
    switch (state.style_set) {
    case NpadStyleSet::FullKey:
    case NpadStyleSet::Handheld:
        new_state_entry.attributes |= NpadAttributes::IsWired;
        break;
    case NpadStyleSet::JoyDual:
        new_state_entry.attributes |=
            NpadAttributes::IsLeftConnected | NpadAttributes::IsRightConnected;
        break;
    case NpadStyleSet::JoyLeft:
        new_state_entry.attributes |= NpadAttributes::IsLeftConnected;
        break;
    case NpadStyleSet::JoyRight:
        new_state_entry.attributes |= NpadAttributes::IsRightConnected;
        break;
    case NpadStyleSet::Palma:
        break;
    default:
        unreachable();
    }

    GetLifo().WriteNext(new_state_entry);
}

} // namespace hydra::horizon::services::hid::internal
