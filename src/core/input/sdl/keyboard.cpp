#include "core/input/sdl/keyboard.hpp"

#include "frontend/sdl3/const.hpp"

namespace hydra::input::sdl {

bool Keyboard::IsPressedImpl(Key key) {
#define KEY_CASE(key, sdl_key)                                                 \
    case Key::key:                                                             \
        return st[sdl_key];

    const bool* st = SDL_GetKeyboardState(nullptr);

    switch (key) {
        KEY_CASE(Q, SDL_SCANCODE_Q)
        KEY_CASE(W, SDL_SCANCODE_W)
        KEY_CASE(E, SDL_SCANCODE_E)
        KEY_CASE(R, SDL_SCANCODE_R)
        KEY_CASE(T, SDL_SCANCODE_T)
        KEY_CASE(Y, SDL_SCANCODE_Y)
        KEY_CASE(U, SDL_SCANCODE_U)
        KEY_CASE(I, SDL_SCANCODE_I)
        KEY_CASE(O, SDL_SCANCODE_O)
        KEY_CASE(P, SDL_SCANCODE_P)
        KEY_CASE(A, SDL_SCANCODE_A)
        KEY_CASE(S, SDL_SCANCODE_S)
        KEY_CASE(D, SDL_SCANCODE_D)
        KEY_CASE(F, SDL_SCANCODE_F)
        KEY_CASE(G, SDL_SCANCODE_G)
        KEY_CASE(H, SDL_SCANCODE_H)
        KEY_CASE(J, SDL_SCANCODE_J)
        KEY_CASE(K, SDL_SCANCODE_K)
        KEY_CASE(L, SDL_SCANCODE_L)
        KEY_CASE(Z, SDL_SCANCODE_Z)
        KEY_CASE(X, SDL_SCANCODE_X)
        KEY_CASE(C, SDL_SCANCODE_C)
        KEY_CASE(V, SDL_SCANCODE_V)
        KEY_CASE(B, SDL_SCANCODE_B)
        KEY_CASE(N, SDL_SCANCODE_N)
        KEY_CASE(M, SDL_SCANCODE_M)

        // TODO: numbers

        // TODO: other stuff

        KEY_CASE(ArrowLeft, SDL_SCANCODE_LEFT)
        KEY_CASE(ArrowRight, SDL_SCANCODE_RIGHT)
        KEY_CASE(ArrowUp, SDL_SCANCODE_UP)
        KEY_CASE(ArrowDown, SDL_SCANCODE_DOWN)

        KEY_CASE(Enter, SDL_SCANCODE_RETURN)
        KEY_CASE(Tab, SDL_SCANCODE_TAB)
        KEY_CASE(Backspace, SDL_SCANCODE_BACKSPACE)
        KEY_CASE(Space, SDL_SCANCODE_SPACE)

        KEY_CASE(LeftShift, SDL_SCANCODE_LSHIFT)
        KEY_CASE(RightShift, SDL_SCANCODE_RSHIFT)
        KEY_CASE(LeftControl, SDL_SCANCODE_LCTRL)
        KEY_CASE(RightControl, SDL_SCANCODE_RCTRL)
        KEY_CASE(LeftAlt, SDL_SCANCODE_LALT)
        KEY_CASE(RightAlt, SDL_SCANCODE_RALT)
        // TODO: correct?
        KEY_CASE(LeftSuper, SDL_SCANCODE_LGUI)
        KEY_CASE(RightSuper, SDL_SCANCODE_RGUI)
    default:
        LOG_NOT_IMPLEMENTED(Input, "Key {}", key);
        return false;
    }

#undef KEY_CASE
}

} // namespace hydra::input::sdl
