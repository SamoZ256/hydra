#pragma once

#include "core/input/device_list.hpp"
#include "core/input/profile.hpp"
#include "core/input/state.hpp"

namespace hydra::input {

constexpr usize MAX_FINGER_COUNT = 16;

class DeviceManager {
  public:
    DeviceManager();

    void pumpEvents() { device_list->pumpEvents(); }

    NpadState pollNpad(horizon::services::hid::internal::NpadIndex index);
    std::map<u32, TouchState> pollTouch();

    // Touch screen devices
    void connectTouchScreenDevice(std::string_view name, IDevice* device) {
        device_list->addDevice(name, device);
    }

    void disconnectTouchScreenDevice(std::string_view name) {
        device_list->removeDevice(name);
    }

  private:
    std::unique_ptr<IDeviceList> device_list;
    std::optional<Profile> profiles[horizon::services::hid::NPAD_COUNT];

    std::map<u64, u32> active_touches;
    usize touch_count{0};
    u16 available_finger_mask{0xffff};

    // Helpers
    u32 beginTouch();
    void endTouch(u32 finger_id);
};

} // namespace hydra::input
