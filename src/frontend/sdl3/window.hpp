#pragma once

#include "core/horizon/ui/handler_base.hpp"
#include "core/system.hpp"
#include "frontend/sdl3/cursor.hpp"

// TODO: if macOS
#include "frontend/native/cocoa/native.hpp"
using hydra::frontend::native::cocoa::Native;

namespace hydra::frontend::sdl3 {

class Context {
  public:
    Context();
    ~Context();
};

class Window : public horizon::ui::IHandler {
  public:
    Window(int argc, const char* argv[]);
    ~Window() override;

    void run();

    // UI
    void showMessageDialog(const horizon::ui::MessageDialogType type,
                           const std::string& title,
                           const std::string& message) override;
    horizon::applets::software_keyboard::SoftwareKeyboardResult
    showSoftwareKeyboard(const std::string& header_text,
                         const std::string& sub_text,
                         const std::string& guide_text,
                         std::string& out_text) override;

  private:
    Context context;

    SDL_Window* window;
    SDL_Renderer* renderer;

    Native native;
    Cursor cursor;

    System system;
    u64 title_id;

    void beginEmulation(const std::string& path);

    void updateWindowTitle();
    void setWindowTitle(const std::string& title);
};

} // namespace hydra::frontend::sdl3
