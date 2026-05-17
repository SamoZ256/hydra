#pragma once

#include "core/input/device_list.hpp"
#include "core/input/profile.hpp"
#include "core/input/state.hpp"

namespace hydra::input {

constexpr usize MAX_FINGER_COUNT = 16;

class DeviceManager {
  public:
    DeviceManager();
    ~DeviceManager();

    NpadState PollNpad(horizon::services::hid::internal::NpadIndex index);
    std::map<u32, TouchState> PollTouch();

    // Touch screen devices
    void ConnectTouchScreenDevice(const std::string& name, IDevice* device) {
        ASSERT(device->ActsAsTouchScreen(), Input,
               "Device \"{}\" does not act as a touch screen", name);
        device_list->devices.insert({name, device});
    }

    IDevice* DisconnectTouchScreenDevice(const std::string& name) {
        auto it = device_list->devices.find(name);
        ASSERT(it != device_list->devices.end(), Input,
               "Touch screen \"{}\" not connected", name);
        device_list->devices.erase(it);
        return it->second;
    }

  private:
    IDeviceList* device_list;
    std::optional<Profile> profiles[horizon::services::hid::NPAD_COUNT];

    std::map<u64, u32> active_touches;
    usize touch_count{0};
    u16 available_finger_mask{0xffff};

    // Helpers
    IDevice* GetDevice(const std::string& name) {
        auto it = device_list->devices.find(name);
        if (it == device_list->devices.end())
            return nullptr;

        return it->second;
    }

    u32 BeginTouch();
    void EndTouch(u32 finger_id);
};

} // namespace hydra::input
