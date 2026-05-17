#pragma once

#include "core/input/controller.hpp"

#include "frontend/sdl3/const.hpp"

namespace hydra::input::sdl {

class Controller : public IController {
  public:
    Controller(SDL_Gamepad* handle_) : handle{handle_} {}
    ~Controller() { SDL_CloseGamepad(handle); }

  protected:
    bool IsPressedImpl(ControllerInput input) override;
    f32 GetAxisValueImpl(ControllerInput input) override;

  private:
    SDL_Gamepad* handle;
};

} // namespace hydra::input::sdl
