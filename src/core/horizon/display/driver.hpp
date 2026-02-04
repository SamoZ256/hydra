#pragma once

#include "core/horizon/display/binder.hpp"
#include "core/horizon/display/display.hpp"

namespace hydra::horizon::display {

class Driver {
  public:
    Driver();

    // Displays
    Display& GetDisplay(handle_id_t id) {
        std::lock_guard lock(display_mutex);
        return *display_pool.Get(id);
    }

    handle_id_t GetDisplayIDFromName(const std::string& name) {
        LOG_NOT_IMPLEMENTED(Horizon, "GetDisplayIDFromName (name: {})", name);

        // HACK
        return 1;
    }

    Display& GetDisplayByName(const std::string& name) {
        return GetDisplay(GetDisplayIDFromName(name));
    }

    // Layers
    u32 CreateLayer(kernel::Process* process, u32 binder_id) {
        std::lock_guard lock(layer_mutex);
        return layer_pool.Add(new Layer(process, binder_id));
    }

    void DestroyLayer(u32 id) {
        std::lock_guard lock(layer_mutex);
        delete layer_pool.Get(id);
        layer_pool.Free(id);
    }

    Layer& GetLayer(u32 id) {
        std::lock_guard lock(layer_mutex);
        return *layer_pool.Get(id);
    }

    // Binders
    u32 CreateBinder() {
        std::lock_guard lock(binder_mutex);
        return binder_pool.Add(new Binder());
    }

    void DestroyBinder(u32 id) {
        std::lock_guard lock(binder_mutex);
        delete binder_pool.Get(id);
        binder_pool.Free(id);
    }

    Binder& GetBinder(u32 id) {
        std::lock_guard lock(binder_mutex);
        return *binder_pool.Get(id);
    }

    // Presenting
    bool AcquirePresentTextures(
        hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer);
    void Present(hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer,
                 hw::tegra_x1::gpu::renderer::ISurfaceCompositor* compositor,
                 u32 width, u32 height);
    void SignalVSync();

    Layer* GetFirstLayerForProcess(kernel::Process* process);

  private:
    std::mutex display_mutex;
    StaticPool<Display*, 8> display_pool;
    std::mutex layer_mutex;
    StaticPool<Layer*, 8> layer_pool;
    std::mutex binder_mutex;
    StaticPool<Binder*, 16> binder_pool;
};

} // namespace hydra::horizon::display
