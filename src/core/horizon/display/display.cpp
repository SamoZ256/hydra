#include "core/horizon/display/display.hpp"

namespace hydra::horizon::display {

bool Display::AcquirePresentTextures(
    std::vector<std::chrono::nanoseconds>& out_dt_list) {
    std::lock_guard lock(mutex);
    bool acquired = false;
    for (u32 id = 1; id < layer_pool.GetCapacity() + 1; id++) {
        if (!layer_pool.IsValid(id))
            continue;
        acquired =
            acquired || layer_pool.Get(id)->AcquirePresentTexture(out_dt_list);
    }

    return acquired;
}

void Display::Present(u32 width, u32 height) {
    // Signal V-Sync
    vsync_event->Signal();

    // Layers
    {
        std::lock_guard lock(mutex);
        for (u32 id = 1; id < layer_pool.GetCapacity() + 1; id++) {
            if (!layer_pool.IsValid(id))
                continue;
            layer_pool.Get(id)->Present(width, height);
        }
    }
}

} // namespace hydra::horizon::display
