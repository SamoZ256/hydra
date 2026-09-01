#pragma once

#include "core/horizon/display/binder.hpp"
#include "core/horizon/display/display.hpp"
#include "core/horizon/handle_pool.hpp"

namespace hydra::horizon::display {

class Driver {
  public:
    Driver(System& system_);

    // Displays
    Display& GetDisplay(Handle handle) {
        std::scoped_lock lock(display_mutex);
        ZTD_ASSIGN_OR(auto display, display_pool.Get(handle),
                      LOG_FATAL(Horizon, "Failed to get display {}", handle));
        return *display;
    }

    Handle GetDisplayIDFromName(const std::string& name) {
        (void)this;
        LOG_NOT_IMPLEMENTED(Horizon, "GetDisplayIDFromName (name: {})", name);

        // HACK
        return 1;
    }

    Display& GetDisplayByName(const std::string& name) {
        return GetDisplay(GetDisplayIDFromName(name));
    }

    // Layers
    Handle CreateLayer(kernel::Process* process, Handle binder_handle) {
        std::scoped_lock lock(layer_mutex);
        return layer_pool.Insert(std::ref(system), process, binder_handle)
            .value();
    }

    void DestroyLayer(Handle handle) {
        std::scoped_lock lock(layer_mutex);
        ASSERT_DEBUG(layer_pool.Free(handle), Horizon, "Invalid layer {}",
                     handle);
    }

    Layer& GetLayer(Handle handle) {
        std::scoped_lock lock(layer_mutex);
        ZTD_ASSIGN_OR(auto layer, layer_pool.Get(handle),
                      LOG_FATAL(Horizon, "Failed to get layer {}", handle));
        return *layer;
    }

    // Binders
    Handle CreateBinder() {
        std::scoped_lock lock(binder_mutex);
        return binder_pool.Insert().value();
    }

    void DestroyBinder(Handle handle) {
        std::scoped_lock lock(binder_mutex);
        ASSERT_DEBUG(binder_pool.Free(handle), Horizon, "Invalid binder {}",
                     handle);
    }

    Binder& GetBinder(Handle handle) {
        std::scoped_lock lock(binder_mutex);
        ZTD_ASSIGN_OR(auto binder, binder_pool.Get(handle),
                      LOG_FATAL(Horizon, "Failed to get binder {}", handle));
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
    StaticHandlePool<Display, 8> display_pool;
    std::mutex layer_mutex;
    StaticHandlePool<Layer, 8> layer_pool;
    std::mutex binder_mutex;
    StaticHandlePool<Binder, 16> binder_pool;
};

} // namespace hydra::horizon::display
