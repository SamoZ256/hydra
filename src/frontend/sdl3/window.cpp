#include "frontend/sdl3/window.hpp"

#include "core/input/device_manager.hpp"

namespace hydra::frontend::sdl3 {

Window::Window(int argc, const char* argv[]) : emulation_context(*this) {
    // SLD3 initialization
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_FATAL(SDL3Window, "Failed to initialize SDL3: {}", SDL_GetError());
        return;
    }

    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
    SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1");

    if (!SDL_CreateWindowAndRenderer(APP_NAME, 1280, 720, SDL_WINDOW_RESIZABLE,
                                     &window, &renderer)) {
        LOG_FATAL(SDL3Window, "Failed to create SDL3 window/renderer: {}",
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
    INPUT_DEVICE_MANAGER_INSTANCE.DisconnectTouchScreenDevice("cursor");

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
            default:
                cursor.Poll(e);
                break;
            }
        }

        if (emulation_context.IsRunning()) {
            // Present
            i32 width, height;
            SDL_GetWindowSize(window, &width, &height);
            bool dt_average_updated;
            emulation_context.ProgressFrame(width, height, dt_average_updated);

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

void Window::BeginEmulation(const std::string& rom_filename) {
    // Connect cursor as a touch screen device
    INPUT_DEVICE_MANAGER_INSTANCE.ConnectTouchScreenDevice("cursor", &cursor);

    // Begin emulation
    emulation_context.SetSurface(SDL_GetRenderMetalLayer(renderer));
    emulation_context.LoadRom(rom_filename);
    emulation_context.Run();
}

void Window::UpdateWindowTitle() {
    const auto dt = emulation_context.GetLastDeltaTimeAverage();
    std::string fps_str;
    if (dt == 0.0f)
        fps_str = "0";
    else
        fps_str = std::to_string((u32)std::round(1.0f / dt));

    // TODO: title name
    const auto title =
        fmt::format("Hydra | TODO(TITLE_NAME) - 0x{:016x} | {} | {} FPS",
                    emulation_context.GetTitleID(),
                    Config::GetInstance().GetGpuRenderer(), fps_str);
    SetWindowTitle(title);
}

void Window::SetWindowTitle(const std::string& title) {
    SDL_SetWindowTitle(window, title.c_str());
}

} // namespace hydra::frontend::sdl3
