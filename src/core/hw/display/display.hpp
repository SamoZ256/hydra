#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/hw/display/layer.hpp"

namespace hydra::hw::display {

// TODO: let a different class manage the event

class Display {
  public:
    void Open() {
        // TODO: autoclear event?
        vsync_event =
            new horizon::kernel::HandleWithId(new horizon::kernel::Event());
    }
    void Close() { delete vsync_event; }

    u32 CreateLayer(u32 binder_id) {
        u32 id = layers.size();
        layers.push_back(new Layer(binder_id));

        return id;
    }

    // TODO: make this more sophisticated
    Layer* GetPresentableLayer() {
        if (layers.empty())
            return nullptr;

        return layers.back();
    }

    // Getters
    horizon::kernel::HandleWithId<horizon::kernel::Event>& GetVSyncEvent() {
        ASSERT_DEBUG(vsync_event, Other, "Invalid V-Sync event");
        return *vsync_event;
    }

    Layer* GetLayer(u32 id) { return layers[id]; }

    bool IsOpen() const { return vsync_event != nullptr; }

  private:
    horizon::kernel::HandleWithId<horizon::kernel::Event>* vsync_event{nullptr};

    std::vector<Layer*> layers;
};

} // namespace hydra::hw::display
