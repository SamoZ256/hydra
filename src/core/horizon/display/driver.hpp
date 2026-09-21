#pragma once

#include "core/horizon/display/binder.hpp"
#include "core/horizon/display/display.hpp"
#include "core/horizon/handle_pool.hpp"

namespace hydra::horizon::display {

class Driver {
  public:
    explicit Driver(System& system_);

    // Displays
    Display& getDisplay(Handle handle) {
        std::scoped_lock lock(display_mutex);
        ZTD_ASSIGN_OR(auto display, display_pool.get(handle),
                      LOG_FATAL(Horizon, "Failed to get display {}", handle));
        return *display;
    }

    Handle getDisplayIdFromName(const std::string& name) {
        (void)this;
        LOG_NOT_IMPLEMENTED(Horizon, "GetDisplayIDFromName (name: {})", name);

        // HACK
        return 1;
    }

    Display& getDisplayByName(const std::string& name) {
        return getDisplay(getDisplayIdFromName(name));
    }

    // Layers
    Handle createLayer(kernel::Process* process, Handle binder_handle) {
        std::scoped_lock lock(layer_mutex);
        return layer_pool.insert(std::ref(system), process, binder_handle)
            .value();
    }

    void destroyLayer(Handle handle) {
        std::scoped_lock lock(layer_mutex);
        ASSERT_DEBUG(layer_pool.free(handle), Horizon, "Invalid layer {}",
                     handle);
    }

    Layer& getLayer(Handle handle) {
        std::scoped_lock lock(layer_mutex);
        ZTD_ASSIGN_OR(auto layer, layer_pool.get(handle),
                      LOG_FATAL(Horizon, "Failed to get layer {}", handle));
        return *layer;
    }

    // Binders
    Handle createBinder() {
        std::scoped_lock lock(binder_mutex);
        return binder_pool.insert().value();
    }

    void destroyBinder(Handle handle) {
        std::scoped_lock lock(binder_mutex);
        ASSERT_DEBUG(binder_pool.free(handle), Horizon, "Invalid binder {}",
                     handle);
    }

    Binder& getBinder(Handle handle) {
        std::scoped_lock lock(binder_mutex);
        ZTD_ASSIGN_OR(auto binder, binder_pool.get(handle),
                      LOG_FATAL(Horizon, "Failed to get binder {}", handle));
        return *binder;
    }

    // Presenting
    bool acquirePresentTextures(
        hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer);
    void present(hw::tegra_x1::gpu::renderer::ICommandBuffer* command_buffer,
                 hw::tegra_x1::gpu::renderer::ISurfaceCompositor* compositor,
                 u32 width, u32 height);
    void signalVSync();

    Layer* getFirstLayerForProcess(kernel::Process* process);

  private:
    System& system;

    std::mutex display_mutex;
    StaticHandlePool<Display, 8> display_pool;
    std::mutex layer_mutex;
    StaticHandlePool<Layer, 8> layer_pool;
    std::mutex binder_mutex;
    StaticHandlePool<Binder, 16> binder_pool;
};

} // namespace hydra::horizon::display
