#pragma once

#include "core/horizon/display/layer.hpp"
#include "core/horizon/kernel/kernel.hpp"

namespace hydra::horizon::display {

class Display {
  public:
    Display() : vsync_event{new kernel::Event(false, "V-Sync event")} {}

    // TODO
    void open() {}
    void close() {}

  private:
    kernel::Event* vsync_event;
    // TODO: name and resolution

  public:
    GETTER(vsync_event, getVSyncEvent);
};

} // namespace hydra::horizon::display
