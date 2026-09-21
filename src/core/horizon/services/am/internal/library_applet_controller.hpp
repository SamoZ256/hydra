#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/am/storage.hpp"

namespace hydra::horizon::services::am::internal {

class StorageQueue {
  public:
    StorageQueue() noexcept = default;
    ~StorageQueue() noexcept {
        for (auto data : queue)
            data->release();
    }

    ZTD_MAKE_NON_COPYABLE(StorageQueue);
    ZTD_MAKE_DEFAULT_MOVABLE(StorageQueue);

    void pushData(IStorage* data) {
        data->retain();
        queue.push_back(data);
    }

    IStorage* popData() {
        ASSERT(pop_index < queue.size(), Services, "No data");
        const auto data = queue[pop_index++];

        return data;
    }

  private:
    std::vector<IStorage*> queue;
    u32 pop_index{0};
};

class LibraryAppletController {
  public:
    explicit LibraryAppletController(const LibraryAppletMode mode_) noexcept
        : mode{mode_}, state_changed_event(std::make_unique<kernel::Event>(
                           false, "Library applet state changed event")),
          interactive_in_data_event(std::make_unique<kernel::Event>(
              false, "Library applet interactive in data event")),
          interactive_out_data_event(std::make_unique<kernel::Event>(
              false, "Library applet interactive out data event")) {}

    ZTD_MAKE_NON_COPYABLE(LibraryAppletController);
    ZTD_MAKE_DEFAULT_MOVABLE(LibraryAppletController);

    // Data

    // In
    void pushInData(IStorage* data) { in_data.pushData(data); }
    IStorage* popInData() { return in_data.popData(); }

    // Out
    void pushOutData(IStorage* data) { out_data.pushData(data); }
    IStorage* popOutData() { return out_data.popData(); }

    // Interactive in
    void pushInteractiveInData(IStorage* data) {
        interactive_in_data.pushData(data);
        interactive_in_data_event->signal();
    }
    IStorage* popInteractiveInData() { return interactive_in_data.popData(); }

    // Interactive out
    void pushInteractiveOutData(IStorage* data) {
        interactive_out_data.pushData(data);
        interactive_out_data_event->signal();
    }
    IStorage* popInteractiveOutData() { return interactive_out_data.popData(); }

    // Events
    kernel::Event& getStateChangedEvent() { return *state_changed_event; }

    kernel::Event& getInteractiveInDataEvent() {
        return *interactive_in_data_event;
    }

    kernel::Event& getInteractiveOutDataEvent() {
        return *interactive_out_data_event;
    }

  private:
    // TODO: use
    [[maybe_unused]] LibraryAppletMode mode;

    std::unique_ptr<kernel::Event> state_changed_event;
    std::unique_ptr<kernel::Event> interactive_in_data_event;
    std::unique_ptr<kernel::Event> interactive_out_data_event;

    StorageQueue in_data;
    StorageQueue out_data;
    StorageQueue interactive_in_data;
    StorageQueue interactive_out_data;
};

} // namespace hydra::horizon::services::am::internal
