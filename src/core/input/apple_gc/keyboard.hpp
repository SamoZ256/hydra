#pragma once

#include "core/input/keyboard.hpp"

namespace hydra::input::apple_gc {

class Keyboard : public IKeyboard {
  public:
    explicit Keyboard(id handle_) : handle{handle_} {}

  protected:
    bool isPressedImpl(Key key) override;

  private:
    id handle;
};

} // namespace hydra::input::apple_gc
