#include "core/input/sdl/device_list.hpp"

#include "core/input/sdl/controller.hpp"
#include "core/input/sdl/keyboard.hpp"
#include "frontend/sdl3/const.hpp"

namespace hydra::input::sdl {

namespace {

bool ewWrapper(void* userdata, SDL_Event* e) {
    auto o = static_cast<DeviceList*>(userdata);
    o->eventWatcher(e);
    return false;
}

} // namespace

DeviceList::DeviceList() {
    has_frontend = SDL_WasInit(SDL_INIT_VIDEO) != 0;

    // Initialize
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        LOG_FATAL(SDL3Window, "Failed to initialize SDL: {}", SDL_GetError());
    }

    // Get initial keyboards
    int kb_count = 0;
    SDL_KeyboardID* keyboards = SDL_GetKeyboards(&kb_count);
    if (keyboards != nullptr) {
        keyboard_count = static_cast<u32>(kb_count);
        if (keyboard_count > 0)
            connectGenericKeyboard();

        SDL_free(keyboards);
    } else {
        keyboard_count = 0;
    }

    // Get initial gamepads
    int gp_count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&gp_count);
    if (gamepads != nullptr) {
        for (int i = 0; i < gp_count; i++)
            connectController(gamepads[i]);

        SDL_free(gamepads);
    }

    // Register event watcher
    SDL_AddEventWatch(ewWrapper, this);
}

DeviceList::~DeviceList() {
    SDL_RemoveEventWatch(ewWrapper, this);
    SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

void DeviceList::pumpEvents() {
    // If there is no frontend event loop, events must be pumped manually
    if (!has_frontend)
        SDL_PumpEvents();
}

void DeviceList::eventWatcher(SDL_Event* e) {
    switch (e->type) {
    case SDL_EVENT_KEYBOARD_ADDED: {
        if (keyboard_count++ == 0)
            connectGenericKeyboard();
        keyboard_count++;
        break;
    }
    case SDL_EVENT_KEYBOARD_REMOVED: {
        if (--keyboard_count == 0)
            removeDevice("Generic Keyboard");
        break;
    }
    case SDL_EVENT_GAMEPAD_ADDED: {
        connectController(e->gdevice.which);
        break;
    }
    case SDL_EVENT_GAMEPAD_REMOVED: {
        SDL_Gamepad* gp = SDL_GetGamepadFromID(e->gdevice.which);
        std::string name = SDL_GetGamepadName(gp);
        removeDevice(name);
        break;
    }
    default:
        break;
    }
}

void DeviceList::connectGenericKeyboard() {
    addDevice("Generic Keyboard", new Keyboard());
}

void DeviceList::connectController(SDL_JoystickID id) {
    SDL_Gamepad* gp = SDL_OpenGamepad(id);
    if (gp != nullptr) {
        std::string name = SDL_GetGamepadName(gp);
        addDevice(name, new Controller(gp));
    } else {
        LOG_ERROR(Input, "Failed to get controller: {}", SDL_GetError());
    }
}

} // namespace hydra::input::sdl
