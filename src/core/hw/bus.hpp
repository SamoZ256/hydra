#pragma once

namespace hydra::hw {

namespace display {
class Display;
}

class Bus {
  public:
    void ConnectDisplay(display::Display* display, u32 display_id) {
        displays[display_id] = display;
    }

    // Getters
    display::Display* GetDisplay(u32 display_id) const {
        return displays[display_id];
    }

  private:
    display::Display*
        displays[2]; // TODO: what's the maximum number of displays?
};

} // namespace hydra::hw
