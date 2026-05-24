#include "core/input/profile.hpp"

#include "core/input/controller.hpp"
#include "core/input/device_manager.hpp"
#include "core/input/keyboard.hpp"

ENABLE_ENUM_FORMATTING_AND_CASTING(
    hydra::horizon::services::hid, NpadButtons, A, "a", B, "b", X, "x", Y, "y",
    StickL, "stick_l", StickR, "stick_r", L, "l", R, "r", ZL, "zl", ZR, "zr",
    Plus, "plus", Minus, "minus", Left, "left", Up, "up", Right, "right", Down,
    "down", StickLLeft, "stick_l_left", StickLUp, "stick_l_up", StickLRight,
    "stick_l_right", StickLDown, "stick_l_down", StickRLeft, "stick_r_left",
    StickRUp, "stick_r_up", StickRRight, "stick_r_right", StickRDown,
    "stick_r_down", LeftSL, "left_sl", LeftSR, "left_sr", RightSL, "right_sl",
    RightSR, "right_sr", Palma, "palma", Verification, "verification",
    HandheldLeftB, "handheld_left_b", LagonCLeft, "lagon_c_left", LagonCUp,
    "lagon_c_up", LagonCRight, "lagon_c_right", LagonCDown, "lagon_c_down")

namespace hydra::input {

namespace {

std::optional<u32> ToValue(DeviceType type, const std::string_view value_str) {
    switch (type) {
    case DeviceType::Keyboard: {
        const auto key_opt = ToKey(value_str);
        return key_opt.has_value()
                   ? std::optional{static_cast<u32>(key_opt.value())}
                   : std::nullopt;
    }
    case DeviceType::Controller: {
        const auto controller_input_opt = ToControllerInput(value_str);
        return controller_input_opt.has_value()
                   ? std::optional{static_cast<u32>(
                         controller_input_opt.value())}
                   : std::nullopt;
    }
    case DeviceType::Cursor:
        LOG_ERROR(Input, "Cursor device does not support codes");
        return invalid<u32>();
    default:
        unreachable();
    }
}

std::string ValueToString(DeviceType device_type, u32 value) {
    switch (device_type) {
    case DeviceType::Keyboard:
        return fmt::format("{}", Key(value));
    case DeviceType::Controller:
        return fmt::format("{}", ControllerInput(value));
    case DeviceType::Cursor:
        LOG_ERROR(Input, "Cursor device does not support codes");
        return {};
    default:
        unreachable();
    }
}

std::optional<Code> ToCode(const std::string_view str) {
    const auto slash_pos = str.find("/");
    if (slash_pos == std::string::npos) {
        LOG_ERROR(Input, "Invalid input code format: {}", str);
        return std::nullopt;
    }

    // Device type
    const auto device_type_str = str.substr(0, slash_pos);
    const auto device_type = ToDeviceType(device_type_str);
    if (device_type == std::nullopt) {
        LOG_ERROR(Input, "Invalid device type: {}", device_type_str);
        return std::nullopt;
    }

    // Value
    const auto value_str = str.substr(slash_pos + 1);
    const auto value = ToValue(device_type.value(), value_str);
    if (value == std::nullopt) {
        LOG_ERROR(Input, "Invalid value: {}", value_str);
        return std::nullopt;
    }

    return Code(device_type.value(), value.value());
}

AnalogStickAxis ToAnalogStickAxis(const std::string_view str) {
    // TODO: clean this up?
    if (str == "l_right") {
        return {true, AnalogStickDirection::Right};
    } else if (str == "l_left") {
        return {true, AnalogStickDirection::Left};
    } else if (str == "l_up") {
        return {true, AnalogStickDirection::Up};
    } else if (str == "l_down") {
        return {true, AnalogStickDirection::Down};
    } else if (str == "r_right") {
        return {false, AnalogStickDirection::Right};
    } else if (str == "r_left") {
        return {false, AnalogStickDirection::Left};
    } else if (str == "r_up") {
        return {false, AnalogStickDirection::Up};
    } else if (str == "r_down") {
        return {false, AnalogStickDirection::Down};
    } else {
        LOG_ERROR(Input, "Invalid analog stick axis \"{}\"", str);
        return {};
    }
}

std::string AnalogStickDirectionToString(const AnalogStickDirection& dir) {
    switch (dir) {
    case AnalogStickDirection::Right:
        return "right";
    case AnalogStickDirection::Left:
        return "left";
    case AnalogStickDirection::Up:
        return "up";
    case AnalogStickDirection::Down:
        return "down";
    }
}

std::string AnalogStickAxisToString(const AnalogStickAxis& axis) {
    return fmt::format(
        "{}_{}", axis.is_left ? "l" : "r",
        hydra::input::AnalogStickDirectionToString(axis.direction));
}

} // namespace

} // namespace hydra::input

namespace toml {

template <>
struct from<hydra::input::Code> {
    template <typename TC>
    static hydra::input::Code from_toml(const basic_value<TC>& v) {
        const auto str = v.as_string();
        return to_code(str);
    }
};

template <>
struct into<hydra::input::Code> {
    template <typename TC>
    static basic_value<TC> into_toml(const hydra::input::Code& obj) {
        return fmt::format(
            "{}/{}", obj.GetDeviceType(),
            hydra::input::ValueToString(obj.GetDeviceType(), obj.GetValue()));
    }
};

template <>
struct from<hydra::input::AnalogStickAxis> {
    template <typename TC>
    static hydra::input::AnalogStickAxis from_toml(const basic_value<TC>& v) {
        const auto str = v.as_string();
        return hydra::input::ToAnalogStickAxis(str);
    }
};

template <>
struct into<hydra::input::AnalogStickAxis> {
    template <typename TC>
    static basic_value<TC>
    into_toml(const hydra::input::AnalogStickAxis& axis) {
        return hydra::input::AnalogStickAxisToString(axis);
    }
};

} // namespace toml

namespace hydra::input {

Profile::Profile(horizon::services::hid::internal::NpadIndex index_,
                 std::string_view name_)
    : index{index_}, name{name_} {
    const auto path = GetProfilesPath();
    if (!std::filesystem::exists(path))
        std::filesystem::create_directory(path);

    Deserialize();
}

void Profile::LoadDefaults() {
    switch (index) {
    case horizon::services::hid::internal::NpadIndex::No1: {
        // Devices
#ifdef PLATFORM_MACOS
        device_names = {"Generic Keyboard"};
#elif defined(PLATFORM_IOS)
        device_names = {"Apple Touch Controller"};
#endif

        // Buttons
        button_mappings = {
            // Controller
            {Code(DeviceType::Controller, ControllerInput::Plus),
             horizon::services::hid::NpadButtons::Plus},
            {Code(DeviceType::Controller, ControllerInput::Minus),
             horizon::services::hid::NpadButtons::Minus},
            {Code(DeviceType::Controller, ControllerInput::Left),
             horizon::services::hid::NpadButtons::Left},
            {Code(DeviceType::Controller, ControllerInput::Right),
             horizon::services::hid::NpadButtons::Right},
            {Code(DeviceType::Controller, ControllerInput::Up),
             horizon::services::hid::NpadButtons::Up},
            {Code(DeviceType::Controller, ControllerInput::Down),
             horizon::services::hid::NpadButtons::Down},
            {Code(DeviceType::Controller, ControllerInput::A),
             horizon::services::hid::NpadButtons::A},
            {Code(DeviceType::Controller, ControllerInput::B),
             horizon::services::hid::NpadButtons::B},
            {Code(DeviceType::Controller, ControllerInput::X),
             horizon::services::hid::NpadButtons::X},
            {Code(DeviceType::Controller, ControllerInput::Y),
             horizon::services::hid::NpadButtons::Y},
            {Code(DeviceType::Controller, ControllerInput::L),
             horizon::services::hid::NpadButtons::L},
            {Code(DeviceType::Controller, ControllerInput::R),
             horizon::services::hid::NpadButtons::R},
            {Code(DeviceType::Controller, ControllerInput::ZL),
             horizon::services::hid::NpadButtons::ZL},
            {Code(DeviceType::Controller, ControllerInput::ZR),
             horizon::services::hid::NpadButtons::ZR},

            // Keyboard
            {Code(DeviceType::Keyboard, Key::Enter),
             horizon::services::hid::NpadButtons::Plus},
            {Code(DeviceType::Keyboard, Key::Tab),
             horizon::services::hid::NpadButtons::Minus},
            {Code(DeviceType::Keyboard, Key::ArrowLeft),
             horizon::services::hid::NpadButtons::Left},
            {Code(DeviceType::Keyboard, Key::ArrowRight),
             horizon::services::hid::NpadButtons::Right},
            {Code(DeviceType::Keyboard, Key::ArrowUp),
             horizon::services::hid::NpadButtons::Up},
            {Code(DeviceType::Keyboard, Key::ArrowDown),
             horizon::services::hid::NpadButtons::Down},
            {Code(DeviceType::Keyboard, Key::L),
             horizon::services::hid::NpadButtons::A},
            {Code(DeviceType::Keyboard, Key::K),
             horizon::services::hid::NpadButtons::B},
            {Code(DeviceType::Keyboard, Key::I),
             horizon::services::hid::NpadButtons::X},
            {Code(DeviceType::Keyboard, Key::J),
             horizon::services::hid::NpadButtons::Y},
            {Code(DeviceType::Keyboard, Key::U),
             horizon::services::hid::NpadButtons::L},
            {Code(DeviceType::Keyboard, Key::O),
             horizon::services::hid::NpadButtons::R},
            {Code(DeviceType::Keyboard, Key::Y),
             horizon::services::hid::NpadButtons::ZL},
            {Code(DeviceType::Keyboard, Key::P),
             horizon::services::hid::NpadButtons::ZR},
        };

        // Analog sticks
        analog_mappings = {
            // Controller
            {Code(DeviceType::Controller, ControllerInput::StickLRight),
             {true, AnalogStickDirection::Right}},
            {Code(DeviceType::Controller, ControllerInput::StickLLeft),
             {true, AnalogStickDirection::Left}},
            {Code(DeviceType::Controller, ControllerInput::StickLUp),
             {true, AnalogStickDirection::Up}},
            {Code(DeviceType::Controller, ControllerInput::StickLDown),
             {true, AnalogStickDirection::Down}},
            {Code(DeviceType::Controller, ControllerInput::StickRRight),
             {false, AnalogStickDirection::Right}},
            {Code(DeviceType::Controller, ControllerInput::StickRLeft),
             {false, AnalogStickDirection::Left}},
            {Code(DeviceType::Controller, ControllerInput::StickRUp),
             {false, AnalogStickDirection::Up}},
            {Code(DeviceType::Controller, ControllerInput::StickRDown),
             {false, AnalogStickDirection::Down}},

            // Keyboard
            {Code(DeviceType::Keyboard, Key::D),
             {true, AnalogStickDirection::Right}},
            {Code(DeviceType::Keyboard, Key::A),
             {true, AnalogStickDirection::Left}},
            {Code(DeviceType::Keyboard, Key::W),
             {true, AnalogStickDirection::Up}},
            {Code(DeviceType::Keyboard, Key::S),
             {true, AnalogStickDirection::Down}},
        };

        break;
    }
    default:
        break;
    }
}

void Profile::Serialize() {
    // TODO: check if changed?

    // TODO: why is the order of everything reversed in the saved config?

    std::ofstream config_file(GetPath());
    if (!config_file.is_open()) {
        LOG_ERROR(Common, "Failed to open npad config file");
        return;
    }

    toml::value data(toml::table{
        {"Buttons", toml::table{}},
        {"AnalogSticks", toml::table{}},
    });

    // Devices
    {
        auto& devices_arr = data["devices"];
        devices_arr = toml::array{};
        devices_arr.as_array().assign(device_names.begin(), device_names.end());
    }

    // Buttons
    {
        auto& buttons = data.at("Buttons");
        for (const auto& mapping : button_mappings) {
            const auto npad_buttons_str =
                fmt::format("{}", mapping.npad_buttons);
            bool has_entry = buttons.contains(npad_buttons_str);
            auto& button = buttons[npad_buttons_str];
            if (!has_entry)
                button = toml::array{};
            button.as_array().push_back(mapping.code);
        }
    }

    // Analog sticks
    {
        auto& analog = data.at("AnalogSticks");
        for (const auto& mapping : analog_mappings) {
            const auto axis_str = AnalogStickAxisToString(mapping.axis);
            bool has_entry = analog.contains(axis_str);
            auto& axis = analog[axis_str];
            if (!has_entry)
                axis = toml::array{};
            axis.as_array().push_back(mapping.code);
        }
    }

    config_file << toml::format(data);
    config_file.close();
}

void Profile::Deserialize() {
    const std::string path = GetPath();

    // Check if exists
    bool exists = std::filesystem::exists(path);
    if (!exists) {
        LoadDefaults();
        Serialize();
        return;
    }

    auto data = toml::parse(path);

    // Devices
    {
        device_names =
            toml::find_or<std::vector<std::string>>(data, "devices", {});
    }

    // Buttons
    if (data.contains("Buttons")) {
        const auto& buttons = data.at("Buttons");
        for (const auto& mappings : buttons.as_table()) {
            const auto button =
                horizon::services::hid::ToNpadButtons(mappings.first);
            if (!button)
                continue;

            for (const auto& mapping : mappings.second.as_array()) {
                const auto& code = ToCode(mapping.as_string());
                if (!code)
                    continue;

                button_mappings.push_back({code.value(), button.value()});
            }
        }
    }

    // Analog sticks
    if (data.contains("AnalogSticks")) {
        const auto& analog = data.at("AnalogSticks");
        for (const auto& mappings : analog.as_table()) {
            const auto axis = ToAnalogStickAxis(mappings.first);
            for (const auto& mapping : mappings.second.as_array()) {
                const auto& code = ToCode(mapping.as_string());
                if (!code)
                    continue;

                analog_mappings.push_back({code.value(), axis});
            }
        }
    }
}

} // namespace hydra::input
