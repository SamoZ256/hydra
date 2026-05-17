#pragma once

#include "core/input/device_list.hpp"

namespace hydra::input::apple_gc {

class DeviceList : public IDeviceList {
  public:
    DeviceList();
    ~DeviceList() override;

    void AddController(id controller);
    void RemoveController(id controller);
    void AddKeyboard(id keyboard);
    void RemoveKeyboard(id keyboard);

  private:
    id impl;
};

} // namespace hydra::input::apple_gc
