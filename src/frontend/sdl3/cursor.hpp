#pragma once

#include "core/input/cursor.hpp"
#include "frontend/sdl3/const.hpp"

namespace hydra::frontend::sdl3 {

class Cursor : public input::ICursor {
  public:
    void poll(SDL_Event e);

    // Touch screen
    u64 getNextBeganTouchId() override;
    u64 getNextEndedTouchId() override;
    void getTouchPosition(u64 id, i32& out_x, i32& out_y) override;

  private:
    bool just_began{false};
    bool just_ended{false};
};

} // namespace hydra::frontend::sdl3
