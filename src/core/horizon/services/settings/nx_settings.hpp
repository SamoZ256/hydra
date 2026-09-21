#pragma once

namespace hydra::horizon::services::settings {

enum class SettingDataType {
    String,
    Integer,
    Boolean,
};

struct SettingValue {
    SettingDataType type;
    union {
        std::string_view s;
        i32 i;
        bool b;
    };

    // NOLINTBEGIN(cppcoreguidelines-explicit-constructor)
    SettingValue(std::string_view value)
        : type(SettingDataType::String), s(value) {}
    SettingValue(i32 value) : type(SettingDataType::Integer), i(value) {}
    SettingValue(bool value) : type(SettingDataType::Boolean), b(value) {}
    // NOLINTEND(cppcoreguidelines-explicit-constructor)
};

extern std::map<std::string, SettingValue> nx_settings;

const SettingValue* getSettingsValue(const std::string_view name,
                                     const std::string_view item_key);

} // namespace hydra::horizon::services::settings
