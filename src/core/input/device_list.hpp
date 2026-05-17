#pragma once

#include "core/input/device.hpp"

namespace hydra::input {

class IDeviceList {
  public:
    virtual ~IDeviceList() {
        for (auto [name, device] : devices)
            delete device;
    }

    void AddDevice(std::string_view name, IDevice* device) {
        std::scoped_lock lock(mutex);
        const auto res = devices.emplace(name, device);
        ASSERT(res.second, Input, "{} already connected", name);
        LOG_INFO(Input, "Device connected: {}", name);
    }

    void RemoveDevice(std::string_view name) {
        std::scoped_lock lock(mutex);
        const auto it = devices.find(name);
        ASSERT(it != devices.end(), Input, "{} not connected", name);
        delete it->second;
        devices.erase(it);
        LOG_INFO(Input, "Device disconnected: {}", name);
    }

    IDevice* GetDevice(std::string_view name) {
        auto it = devices.find(name);
        if (it == devices.end())
            return nullptr;

        return it->second;
    }

  private:
    std::mutex mutex;
    std::map<std::string, IDevice*, std::less<>> devices;

  public:
    REF_GETTER(mutex, GetMutex);
};

} // namespace hydra::input
