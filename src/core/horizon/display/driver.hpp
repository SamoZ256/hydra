#pragma once

#include "core/horizon/display/binder.hpp"
#include "core/horizon/display/display.hpp"

namespace hydra::horizon::display {

class Driver {
  public:
    Driver(System& system_);

    // Displays
    Display& GetDisplay(handle_id_t id) {
        std::scoped_lock lock(display_mutex);
        ZTD_ASSIGN_OR(auto display, display_pool.get(id),
                      LOG_FATAL(Horizon, "Failed to get display {}", id));
        return *display;
    }

    handle_id_t GetDisplayIDFromName(const std::string& name) {
        (void)this;
        LOG_NOT_IMPLEMENTED(Horizon, "GetDisplayIDFromName (name: {})", name);

        // HACK
        return 1;
    }

    Display& GetDisplayByName(const std::string& name) {
        return GetDisplay(GetDisplayIDFromName(name));
    }

    // Layers
    u32 CreateLayer(kernel::Process* process, u32 binder_id) {
        std::scoped_lock lock(layer_mutex);
        return layer_pool.insert(std::ref(system), process, binder_id)
            .value_or(INVALID_HANDLE_ID);
    }

    void DestroyLayer(u32 id) {
        std::scoped_lock lock(layer_mutex);
        ASSERT_DEBUG(layer_pool.free(id), Horizon, "Invalid layer {}", id);
    }

    Layer& GetLayer(u32 id) {
        std::scoped_lock lock(layer_mutex);
        ZTD_ASSIGN_OR(auto layer, layer_pool.get(id),
                      LOG_FATAL(Horizon, "Failed to get layer {}", id));
        return *layer;
    }

    // Binders
    u32 CreateBinder() {
        std::scoped_lock lock(binder_mutex);
        return binder_pool.insert().value_or(INVALID_HANDLE_ID);
    }

    void DestroyBinder(u32 id) {
        std::scoped_lock lock(binder_mutex);
        ASSERT_DEBUG(binder_pool.free(id), Horizon, "Invalid binder {}", id);
    }

    Binder& GetBinder(u32 id) {
        std::scoped_lock lock(binder_mutex);
        ZTD_ASSIGN_OR(auto binder, binder_pool.get(id),
                      LOG_FATAL(Horizon, "Failed to get binder {}", id));
        return *binder;
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
    System& system;

    std::mutex display_mutex;
    ztd::mem::StaticPool<Display, 8> display_pool;
    std::mutex layer_mutex;
    ztd::mem::StaticPool<Layer, 8> layer_pool;
    std::mutex binder_mutex;
    ztd::mem::StaticPool<Binder, 16> binder_pool;
};

} // namespace hydra::horizon::display
