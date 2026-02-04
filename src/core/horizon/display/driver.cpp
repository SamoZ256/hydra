#include "core/horizon/display/driver.hpp"

#include "core/horizon/os.hpp"

namespace hydra::horizon::display {

Driver::Driver() { display_pool.Add(new Display()); }

bool Driver::AcquirePresentTextures(
    hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer) {
    bool acquired = false;
    {
        std::lock_guard lock(layer_mutex);
        for (u32 layer_id = 1; layer_id < layer_pool.GetCapacity() + 1;
             layer_id++) {
            if (!layer_pool.IsValid(layer_id))
                continue;
            acquired |=
                layer_pool.Get(layer_id)->AcquirePresentTexture(command_buffer);
        }
    }

    return acquired;
}

void Driver::Present(
    hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer,
    hw::tegra_x1::gpu::renderer::ISurfaceCompositor* compositor, u32 width,
    u32 height) {
    std::lock_guard lock(layer_mutex);
    std::vector<Layer*> sorted_layers;
    for (u32 layer_id = 1; layer_id < layer_pool.GetCapacity() + 1;
         layer_id++) {
        if (!layer_pool.IsValid(layer_id))
            continue;

        auto layer = layer_pool.Get(layer_id);

        // Find the correct position
        bool inserted = false;
        for (u32 i = 0; i < sorted_layers.size(); i++) {
            if (sorted_layers[i]->GetZ() > layer->GetZ()) {
                sorted_layers.insert(sorted_layers.begin() + i, layer);
                inserted = true;
                break;
            }
        }
        if (!inserted)
            sorted_layers.push_back(layer);
    }

    // Viewport
    const auto src_size = float2(OS_INSTANCE.GetDisplayResolution());
    auto scale_x = f32(width) / src_size.x();
    auto scale_y = f32(height) / src_size.y();

    FloatRect2D dst_rect;
    float dst_scale;
    if (scale_x > scale_y) {
        dst_scale = scale_y;
        const auto dst_width = src_size.x() * dst_scale;
        dst_rect.origin = {(width - dst_width) / 2.f, 0.f};
        dst_rect.size = {dst_width, f32(height)};
    } else {
        dst_scale = scale_x;
        const auto dst_height = src_size.y() * dst_scale;
        dst_rect.origin = {0.f, (height - dst_height) / 2.f};
        dst_rect.size = {f32(width), dst_height};
    }

    // Present
    for (u32 i = 0; i < sorted_layers.size(); i++)
        sorted_layers[i]->Present(command_buffer, compositor, dst_rect,
                                  dst_scale, i != 0);
}

void Driver::SignalVSync() {
    // NOTE: we signal all displays at once for simplicity
    std::lock_guard lock(display_mutex);
    for (u32 display_id = 1; display_id < layer_pool.GetCapacity() + 1;
         display_id++) {
        if (!display_pool.IsValid(display_id))
            continue;
        display_pool.Get(display_id)->GetVSyncEvent()->Signal();
    }
}

Layer* Driver::GetFirstLayerForProcess(kernel::Process* process) {
    std::lock_guard lock(layer_mutex);
    for (u32 layer_id = 1; layer_id < layer_pool.GetCapacity() + 1;
         layer_id++) {
        if (!layer_pool.IsValid(layer_id))
            continue;

        auto layer = layer_pool.Get(layer_id);
        if (layer->GetProcess() == process)
            return layer;
    }

    return nullptr;
}

} // namespace hydra::horizon::display
