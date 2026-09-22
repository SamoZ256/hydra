#pragma once

#include "core/horizon/services/hid/internal/npad_index.hpp"
#include "core/input/const.hpp"

namespace hydra::input {

class Profile {
  public:
    Profile(horizon::services::hid::internal::NpadIndex index_,
            std::string_view name_);

    void reset() {
        button_mappings = {};
        analog_mappings = {};
    }

    void loadDefaults();

    void serialize();
    void deserialize();

  private:
    horizon::services::hid::internal::NpadIndex index;
    std::string name;

    std::vector<std::string> device_names;
    std::vector<CodeButtonMapping> button_mappings;
    std::vector<CodeAnalogMapping> analog_mappings;

    // Helpers
    static std::string getProfilesPath() {
        return fmt::format("{}/input_profiles",
                           CONFIG_INSTANCE.getAppDataPath());
    }

    std::string getPath() const {
        return fmt::format("{}/{}.toml", getProfilesPath(), name);
    }

  public:
    CONST_REF_GETTER(device_names, getDeviceNames);
    CONST_REF_GETTER(button_mappings, getButtonMappings);
    CONST_REF_GETTER(analog_mappings, getAnalogMappings);
};

} // namespace hydra::input
