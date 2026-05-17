#pragma once

#include "core/input/device.hpp"

namespace hydra::input {

class IDeviceList {
    friend class DeviceManager;

  public:
    virtual ~IDeviceList() {
        for (auto [name, device] : devices)
            delete device;
    }

  protected:
    bool HasDevice(std::string_view name) { return devices.contains(name); }

    void AddDevice(std::string_view name, IDevice* device) {
        const auto res = devices.emplace(name, device);
        ASSERT(res.second, Input, "{} already connected", name);
        LOG_INFO(Input, "Device connected: {}", name);
    }

    void RemoveDevice(std::string_view name) {
        const auto it = devices.find(name);
        ASSERT(it != devices.end(), Input, "{} not connected", name);
        delete it->second;
        devices.erase(it);
        LOG_INFO(Input, "Device disconnected: {}", name);
    }

  private:
    std::map<std::string, IDevice*, std::less<>> devices;
};

} // namespace hydra::input
