#include "frontend/sdl3/window.hpp"

int main(int argc, const char* argv[]) {
    // Window
    hydra::frontend::sdl3::Window window(argc, argv);
    window.run();

    return 0;
}
