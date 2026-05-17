#pragma once

#include "core/input/keyboard.hpp"

namespace hydra::input::sdl {

class Keyboard : public IKeyboard {
  public:
    Keyboard() {}

  protected:
    bool IsPressedImpl(Key key) override;
};

} // namespace hydra::input::sdl
