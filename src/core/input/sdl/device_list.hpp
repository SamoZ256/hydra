#pragma once

#include "core/input/device_list.hpp"
#include "frontend/sdl3/const.hpp"

namespace hydra::input::sdl {

class DeviceList : public IDeviceList {
  public:
    DeviceList();
    ~DeviceList() override;

    void pumpEvents() override;

    void eventWatcher(SDL_Event* e);

  private:
    bool has_frontend; // Indicates whether the an SDL frontend is used
    u32 keyboard_count;

    // Helpers
    void connectGenericKeyboard();
    void connectController(SDL_JoystickID id);
};

} // namespace hydra::input::sdl
