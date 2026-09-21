#pragma once

#include "core/input/keyboard.hpp"

namespace hydra::input::sdl {

class Keyboard : public IKeyboard {
  public:
    Keyboard() = default;

  protected:
    bool isPressedImpl(Key key) override;
};

} // namespace hydra::input::sdl
