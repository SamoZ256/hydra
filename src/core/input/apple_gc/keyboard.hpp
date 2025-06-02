#pragma once

#include "core/input/keyboard_base.hpp"

namespace hydra::input::apple_gc {

class Keyboard : public KeyboardBase {
  public:
    Keyboard(id handle_) : handle{handle_} {}

  protected:
    bool IsPressedImpl(Key key) override;

  private:
    id handle;
};

} // namespace hydra::input::apple_gc
