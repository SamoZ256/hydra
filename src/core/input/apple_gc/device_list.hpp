#pragma once

#include "core/input/device_list.hpp"

namespace hydra::input::apple_gc {

class DeviceList : public IDeviceList {
  public:
    DeviceList();
    ~DeviceList();

    void _AddController(id controller);
    void _RemoveController(id controller);
    void _AddKeyboard(id keyboard);
    void _RemoveKeyboard(id keyboard);

  private:
    id impl;
};

} // namespace hydra::input::apple_gc
