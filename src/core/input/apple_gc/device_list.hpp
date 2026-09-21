#pragma once

#include "core/input/device_list.hpp"

namespace hydra::input::apple_gc {

class DeviceList : public IDeviceList {
  public:
    DeviceList();
    ~DeviceList() override;

    void addController(id controller);
    void removeController(id controller);
    void addKeyboard(id keyboard);
    void removeKeyboard(id keyboard);

  private:
    id impl;
};

} // namespace hydra::input::apple_gc
