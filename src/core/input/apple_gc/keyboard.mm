#include "core/input/apple_gc/keyboard.hpp"

#import <GameController/GameController.h>

namespace hydra::input::apple_gc {

bool Keyboard::IsPressedImpl(Key key) {
#define KEY_CASE(key, gc_key)                                                  \
    case Key::key:                                                             \
        gc_key_code = GCKeyCode##gc_key;                                       \
        break;

    GCKeyCode gc_key_code;
    switch (key) {
        KEY_CASE(Q, KeyQ)
        KEY_CASE(W, KeyW)
        KEY_CASE(E, KeyE)
        KEY_CASE(R, KeyR)
        KEY_CASE(T, KeyT)
        KEY_CASE(Y, KeyY)
        KEY_CASE(U, KeyU)
        KEY_CASE(I, KeyI)
        KEY_CASE(O, KeyO)
        KEY_CASE(P, KeyP)
        KEY_CASE(A, KeyA)
        KEY_CASE(S, KeyS)
        KEY_CASE(D, KeyD)
        KEY_CASE(F, KeyF)
        KEY_CASE(G, KeyG)
        KEY_CASE(H, KeyH)
        KEY_CASE(J, KeyJ)
        KEY_CASE(K, KeyK)
        KEY_CASE(L, KeyL)
        KEY_CASE(Z, KeyZ)
        KEY_CASE(X, KeyX)
        KEY_CASE(C, KeyC)
        KEY_CASE(V, KeyV)
        KEY_CASE(B, KeyB)
        KEY_CASE(N, KeyN)
        KEY_CASE(M, KeyM)

        // TODO: numbers

        // TODO: other stuff

        KEY_CASE(ArrowLeft, LeftArrow)
        KEY_CASE(ArrowRight, RightArrow)
        KEY_CASE(ArrowUp, UpArrow)
        KEY_CASE(ArrowDown, DownArrow)

        KEY_CASE(Enter, ReturnOrEnter)
        KEY_CASE(Tab, Tab)
        KEY_CASE(Backspace, DeleteOrBackspace)
        KEY_CASE(Space, Spacebar)

        KEY_CASE(LeftShift, LeftShift)
        KEY_CASE(RightShift, RightShift)
        KEY_CASE(LeftControl, LeftControl)
        KEY_CASE(RightControl, RightControl)
        KEY_CASE(LeftAlt, LeftAlt)
        KEY_CASE(RightAlt, RightAlt)
        // TODO: correct?
        KEY_CASE(LeftSuper, LeftGUI)
        KEY_CASE(RightSuper, RightGUI)
    default:
        // TODO: don't cast to u32
        LOG_NOT_IMPLEMENTED(Input, "Key {}", (u32)key);
        return false;
    }

    return [[((GCKeyboard*)handle).keyboardInput buttonForKeyCode:gc_key_code]
        isPressed];
}

} // namespace hydra::input::apple_gc
