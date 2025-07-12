#pragma once

#include "core/horizon/display/layer.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon::display {

class Display {
  public:
    Display()
        : vsync_event{new kernel::Event(kernel::EventFlags::AutoClear,
                                        "V-Sync event")} {}

    // TODO
    void Open() {}
    void Close() {}

    u32 CreateLayer(u32 binder_id) {
        u32 id = layers.size();
        layers.push_back(new Layer(binder_id));

        return id;
    }

    // TODO: should draw to a specific layer
    Layer* GetPresentableLayer() {
        if (layers.empty())
            return nullptr;

        return layers.back();
    }

    Layer& GetLayer(u32 id) { return *layers[id]; }

  private:
    std::mutex mutex;
    kernel::Event* vsync_event;

    std::vector<Layer*> layers;

  public:
    REF_GETTER(mutex, GetMutex);
    REF_GETTER(vsync_event, GetVSyncEvent);
};

} // namespace hydra::horizon::display
