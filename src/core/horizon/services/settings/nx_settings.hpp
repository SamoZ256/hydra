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
        const char* s;
        i32 i;
        bool b;
    };

    static SettingValue String(const char* value) {
        return {SettingDataType::String, {.s = value}};
    }

    static SettingValue Integer(i32 value) {
        return {SettingDataType::Integer, {.i = value}};
    }

    static SettingValue Boolean(bool value) {
        return {SettingDataType::Boolean, {.b = value}};
    }
};

extern std::map<std::string, SettingValue> nx_settings;

} // namespace hydra::horizon::services::settings
