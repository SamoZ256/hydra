#include "frontend/sdl3/window.hpp"

#include "core/horizon/loader/loader_base.hpp"
#include "core/input/device_manager.hpp"

namespace hydra::frontend::sdl3 {

Window::Window(int argc, const char* argv[]) : system(*this) {
    // SLD3 initialization
    u32 flags = CONFIG_INSTANCE.GetInputBackend() == InputBackend::Sdl
                    ? SDL_INIT_VIDEO | SDL_INIT_GAMEPAD
                    : SDL_INIT_VIDEO;
    if (!SDL_Init(flags)) {
        LOG_FATAL(SDL3Window, "Failed to initialize SDL3: {}", SDL_GetError());
        return;
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    if (!SDL_CreateWindowAndRenderer(APP_NAME, 1280, 720, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        LOG_FATAL(SDL3Window, "Failed to create window/renderer: {}",
                  SDL_GetError());
        return;
    }

    // Parse arguments
    // TODO: use a parser library
    argv++;
    argc--;
    ASSERT(argc <= 1, SDL3Window, "Expected at most 1 argument, got {}", argc);
    if (argc >= 1)
        BeginEmulation(argv[0]);
}

Window::~Window() {
    system.GetInputDeviceManager().DisconnectTouchScreenDevice("cursor");

    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Window::Run() {
    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_EVENT_QUIT:
                running = false;
                break;
            case SDL_EVENT_DROP_FILE:
                BeginEmulation(e.drop.data);
                break;
            case SDL_EVENT_KEY_DOWN: {
                SDL_Keymod modifiers = SDL_GetModState();
#ifdef __APPLE__
                if (modifiers & SDL_KMOD_GUI) {
#else
                if (modifiers & SDL_KMOD_CTRL) {
#endif
                    if (e.key.key == SDLK_T) {
                        system.TakeScreenshot();
                    } else if (e.key.key == SDLK_O) {
                        auto& handheld_mode = CONFIG_INSTANCE.GetHandheldMode();
                        handheld_mode = !handheld_mode;
                        system.NotifyOperationModeChanged();
                    } else if (e.key.key == SDLK_P) {
                        system.CaptureGpuFrame();
                    }
                }
                break;
            }
            default:
                cursor.Poll(e);
                break;
            }
        }

        if (system.IsRunning()) {
            // Present
            i32 width, height;
            SDL_GetWindowSize(window, &width, &height);
            bool dt_average_updated;
            system.ProgressFrame(static_cast<u32>(width),
                                 static_cast<u32>(height), dt_average_updated);

            // Update window title
            if (dt_average_updated)
                UpdateWindowTitle();
        }
    }
}

void Window::ShowMessageDialog(const horizon::ui::MessageDialogType type,
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
Window::ShowSoftwareKeyboard(const std::string& header_text,
                             const std::string& sub_text,
                             const std::string& guide_text,
                             std::string& out_text) {
    return native.ShowInputTextDialog(header_text, sub_text, guide_text,
                                      out_text)
               ? horizon::applets::software_keyboard::SoftwareKeyboardResult::OK
               : horizon::applets::software_keyboard::SoftwareKeyboardResult::
                     Cancel;
}

void Window::BeginEmulation(const std::string& path) {
    // Connect cursor as a touch screen device
    system.GetInputDeviceManager().ConnectTouchScreenDevice("cursor", &cursor);

    system.SetSurface(SDL_GetRenderMetalLayer(renderer));
    // TODO: support loading applets from firmware
    auto loader = horizon::loader::LoaderBase::CreateFromPath(path);
    system.LoadAndStart(loader);
    title_id = loader->GetTitleID();
    delete loader;
}

void Window::UpdateWindowTitle() {
    const auto dt = system.GetLastDeltaTimeAverage();
    std::string fps_str;
    if (dt == 0.0f)
        fps_str = "0";
    else
        fps_str = std::to_string(static_cast<u32>(std::round(1.0f / dt)));

    // TODO: title name
    const auto title =
        fmt::format("Hydra | TODO(TITLE_NAME) - 0x{:016x} | {} | {} FPS",
                    title_id, CONFIG_INSTANCE.GetGpuRenderer(), fps_str);
    SetWindowTitle(title);
}

void Window::SetWindowTitle(const std::string& title) {
    SDL_SetWindowTitle(window, title.c_str());
}

} // namespace hydra::frontend::sdl3
