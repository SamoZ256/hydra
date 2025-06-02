#pragma once

#include "core/input/device_base.hpp"

namespace hydra::input {

class DeviceListBase {
    friend class DeviceManager;

  public:
    virtual ~DeviceListBase() {
        for (auto [name, device] : devices)
            delete device;
    }

  protected:
    std::map<std::string, DeviceBase*> devices;
};

} // namespace hydra::input
