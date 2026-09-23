#include "frontend/sdl3/cursor.hpp"

namespace hydra::frontend::sdl3 {

namespace {

constexpr u64 SDL3_CURSOR_TOUCH_ID =
    static_cast<u64>(makeMagic4('S', 'D', 'L', '3')) << 32 |
    static_cast<u64>(makeMagic4('C', 'R', 'S', 'R'));
}

void Cursor::poll(SDL_Event e) {
    switch (e.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN:
        just_began = true;
        break;
    case SDL_EVENT_MOUSE_BUTTON_UP:
        just_ended = true;
        break;
    default:
        break;
    }
}

u64 Cursor::getNextBeganTouchId() {
    if (just_began) {
        just_began = false;
        return SDL3_CURSOR_TOUCH_ID;
    }

    return invalid<u64>();
}

u64 Cursor::getNextEndedTouchId() {
    if (just_ended) {
        just_ended = false;
        return SDL3_CURSOR_TOUCH_ID;
    }

    return invalid<u64>();
}

void Cursor::getTouchPosition(u64 id, i32& out_x, i32& out_y) {
    ASSERT_DEBUG(id == SDL3_CURSOR_TOUCH_ID, SDL3Window,
                 "Invalid SDL3 cursor touch id 0x{:016x}", id);
    f32 cursor_x;
    f32 cursor_y;
    SDL_GetMouseState(&cursor_x, &cursor_y);
    out_x = static_cast<i32>(cursor_x);
    out_y = static_cast<i32>(cursor_y);
}

} // namespace hydra::frontend::sdl3
