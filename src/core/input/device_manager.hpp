#pragma once

#include "core/input/device_list.hpp"
#include "core/input/profile.hpp"
#include "core/input/state.hpp"

namespace hydra::input {

constexpr usize MAX_FINGER_COUNT = 16;

class DeviceManager {
  public:
    DeviceManager();

    NpadState PollNpad(horizon::services::hid::internal::NpadIndex index);
    std::map<u32, TouchState> PollTouch();

    // Touch screen devices
    void ConnectTouchScreenDevice(std::string_view name, IDevice* device) {
        device_list->AddDevice(name, device);
    }

    void DisconnectTouchScreenDevice(std::string_view name) {
        device_list->RemoveDevice(name);
    }

  private:
    std::unique_ptr<IDeviceList> device_list;
    std::optional<Profile> profiles[horizon::services::hid::NPAD_COUNT];

    std::map<u64, u32> active_touches;
    usize touch_count{0};
    u16 available_finger_mask{0xffff};

    // Helpers
    u32 BeginTouch();
    void EndTouch(u32 finger_id);
};

} // namespace hydra::input
