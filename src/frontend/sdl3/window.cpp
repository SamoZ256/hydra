#include "frontend/sdl3/window.hpp"

#include "core/horizon/loader/loader.hpp"
#include "core/input/device_manager.hpp"

namespace hydra::frontend::sdl3 {

Context::Context() {
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        LOG_FATAL(SDL3Window, "Failed to initialize SDL: {}", SDL_GetError());
    }
}

Context::~Context() { SDL_Quit(); }

Window::Window(int argc, const char* argv[]) : system(*this) {
    // Window and renderer
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    if (!SDL_CreateWindowAndRenderer(APP_NAME, 1280, 720, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        LOG_FATAL(SDL3Window, "Failed to create window/renderer: {}",
                  SDL_GetError());
    }

    // Parse arguments
    // TODO: use a parser library
    argv++;
    argc--;
    ASSERT(argc <= 1, SDL3Window, "Expected at most 1 argument, got {}", argc);
    if (argc >= 1)
        beginEmulation(argv[0]);
}

Window::~Window() {
    system.getInputDeviceManager().disconnectTouchScreenDevice("cursor");

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
}

void Window::run() {
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_DROP_FILE:
                beginEmulation(e.drop.data);
                break;
            case SDL_EVENT_KEY_DOWN: {
                SDL_Keymod modifiers = SDL_GetModState();
#ifdef __APPLE__
                if ((modifiers & SDL_KMOD_GUI) != 0) {
#else
                if (modifiers & SDL_KMOD_CTRL) {
#endif
                    if (e.key.key == SDLK_T) {
                        system.takeScreenshot();
                    } else if (e.key.key == SDLK_O) {
                        auto& handheld_mode = CONFIG_INSTANCE.getHandheldMode();
                        handheld_mode = !handheld_mode;
                        system.notifyOperationModeChanged();
                    } else if (e.key.key == SDLK_P) {
                        system.captureGpuFrame();
                    }
                }
                break;
            }
            default:
                cursor.poll(e);
                break;
            }
        }

        if (system.isRunning()) {
            // Present
            i32 width;
            i32 height;
            SDL_GetWindowSize(window, &width, &height);
            bool dt_average_updated;
            system.progressFrame(static_cast<u32>(width),
                                 static_cast<u32>(height), dt_average_updated);

            // Update window title
            if (dt_average_updated)
                updateWindowTitle();
        }
    }
}

void Window::showMessageDialog(const horizon::ui::MessageDialogType type,
                               const std::string& title,
                               const std::string& message) {
    SDL_MessageBoxFlags flags = 0;
    switch (type) {
    case horizon::ui::MessageDialogType::Info:
        flags |= SDL_MESSAGEBOX_INFORMATION;
        break;
    case horizon::ui::MessageDialogType::Warning:
        flags |= SDL_MESSAGEBOX_WARNING;
        break;
    case horizon::ui::MessageDialogType::Error:
        flags |= SDL_MESSAGEBOX_ERROR;
        break;
    }

    // TODO: why does this crash?
    SDL_ShowSimpleMessageBox(flags, title.c_str(), message.c_str(), window);
}

horizon::applets::software_keyboard::SoftwareKeyboardResult
Window::showSoftwareKeyboard(const std::string& header_text,
                             const std::string& sub_text,
                             const std::string& guide_text,
                             std::string& out_text) {
    return native.showInputTextDialog(header_text, sub_text, guide_text,
                                      out_text)
               ? horizon::applets::software_keyboard::SoftwareKeyboardResult::OK
               : horizon::applets::software_keyboard::SoftwareKeyboardResult::
                     Cancel;
}

void Window::beginEmulation(const std::string& path) {
    // Create loader
    // TODO: support loading applets from firmware
    // TODO: display error when loading fails
    ZTD_ASSIGN_OR_RETURN(auto loader,
                         horizon::loader::ILoader::createFromPath(path));

    // Connect cursor as a touch screen device
    system.getInputDeviceManager().connectTouchScreenDevice("cursor", &cursor);

    // Start
    system.setSurface(SDL_GetRenderMetalLayer(renderer));
    system.loadAndStart(loader);
    title_id = loader->getTitleId();
}

void Window::updateWindowTitle() {
    const auto dt = system.getLastDeltaTimeAverage();
    std::string fps_str;
    if (dt == 0.0f)
        fps_str = "0";
    else
        fps_str = std::to_string(static_cast<u32>(std::round(1.0f / dt)));

    // TODO: title name
    const auto title =
        fmt::format("Hydra | TODO(TITLE_NAME) - 0x{:016x} | {} | {} FPS",
                    title_id, CONFIG_INSTANCE.getGpuRenderer(), fps_str);
    setWindowTitle(title);
}

void Window::setWindowTitle(const std::string& title) {
    SDL_SetWindowTitle(window, title.c_str());
}

} // namespace hydra::frontend::sdl3
