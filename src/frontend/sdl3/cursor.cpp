#include "frontend/sdl3/cursor.hpp"

namespace hydra::frontend::sdl3 {

namespace {

constexpr u64 SDL3_CURSOR_TOUCH_ID = (u64)make_magic4('S', 'D', 'L', '3')
                                         << 32 |
                                     (u64)make_magic4('C', 'R', 'S', 'R');

}

void Cursor::Poll(SDL_Event e) {
    switch (e.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        pressed = true;
        just_began = true;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        pressed = false;
        just_ended = true;
        break;
    default:
        break;
    }
}

u64 Cursor::GetNextBeganTouchID() {
    if (just_began) {
        just_began = false;
        return SDL3_CURSOR_TOUCH_ID;
    }

    return invalid<u64>();
}

u64 Cursor::GetNextEndedTouchID() {
    if (just_ended) {
        just_ended = false;
        return SDL3_CURSOR_TOUCH_ID;
    }

    return invalid<u64>();
}

void Cursor::GetTouchPosition(u64 id, i32& out_x, i32& out_y) {
    ASSERT_DEBUG(id == SDL3_CURSOR_TOUCH_ID, SDL3Window,
                 "Invalid SDL3 cursor touch id 0x{:016x}", id);
    f32 cursor_x, cursor_y;
    SDL_GetMouseState(&cursor_x, &cursor_y);
    out_x = cursor_x;
    out_y = cursor_y;
}

} // namespace hydra::frontend::sdl3
