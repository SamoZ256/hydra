#pragma once

#include "core/input/cursor_base.hpp"
#include "frontend/sdl3/const.hpp"

namespace hydra::frontend::sdl3 {

class Cursor : public input::CursorBase {
  public:
    void Poll(SDL_Event e);

    // Touch screen
    u64 GetNextBeganTouchID() override;
    u64 GetNextEndedTouchID() override;
    void GetTouchPosition(u64 id, i32& out_x, i32& out_y) override;

  private:
    bool pressed{false};

    bool just_began{false};
    bool just_ended{false};
};

} // namespace hydra::frontend::sdl3
