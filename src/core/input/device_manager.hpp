#pragma once

#include "core/input/device_list_base.hpp"

#define INPUT_DEVICE_MANAGER_INSTANCE input::DeviceManager::GetInstance()

namespace hydra::input {

class DeviceManager {
  public:
    static DeviceManager& GetInstance() {
        static DeviceManager s_instance;
        return s_instance;
    }

    DeviceManager();
    ~DeviceManager();

    void ConnectDevices();

    void Poll();

    // Touch screen devices
    void ConnectTouchScreenDevice(const std::string& name, DeviceBase* device) {
        ASSERT(device->ActsAsTouchScreen(), Input,
               "Device \"{}\" does not act as a touch screen", name);
        device_list->devices.insert({name, device});
    }

    DeviceBase* DisconnectTouchScreenDevice(const std::string& name) {
        auto it = device_list->devices.find(name);
        ASSERT(it != device_list->devices.end(), Input,
               "Touch screen \"{}\" not connected", name);
        device_list->devices.erase(it);
        return it->second;
    }

  private:
    DeviceListBase* device_list;

    std::map<u64, u32> active_touches;

    // Helpers
    DeviceBase* GetDevice(const std::string& name) {
        auto it = device_list->devices.find(name);
        if (it == device_list->devices.end())
            return nullptr;

        return it->second;
    }
};

} // namespace hydra::input
