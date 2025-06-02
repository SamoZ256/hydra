#include "frontend/sdl3/window.hpp"

int main(int argc, const char* argv[]) {
    // Config
    hydra::CONFIG_INSTANCE.Initialize();

    // Window
    hydra::frontend::sdl3::Window window(argc, argv);
    window.Run();

    return 0;
}
