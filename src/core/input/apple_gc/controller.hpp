#pragma once

#include "core/input/controller.hpp"

namespace hydra::input::apple_gc {

class Controller : public IController {
  public:
    explicit Controller(id handle_) : handle{handle_} {}

  protected:
    bool isPressedImpl(ControllerInput input) override;
    f32 getAxisValueImpl(ControllerInput input) override;

  private:
    id handle;
};

} // namespace hydra::input::apple_gc
