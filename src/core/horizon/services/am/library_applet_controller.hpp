#pragma once

#include "core/horizon/const.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/am/storage.hpp"

namespace hydra::horizon::services::am {

class StorageQueue {
  public:
    void PushData(IStorage* data) { queue.push(data); }

    IStorage* PopData() {
        ASSERT(!queue.empty(), Services, "No data");
        const auto data = queue.front();
        queue.pop();

        return data;
    }

  private:
    std::queue<IStorage*> queue;
};

class LibraryAppletController {
  public:
    LibraryAppletController(const LibraryAppletMode mode_)
        : mode{mode_}, state_changed_event{new kernel::Event(
                           kernel::EventFlags::None,
                           "Library applet state changed event")},
          interactive_in_data_event{
              new kernel::Event(kernel::EventFlags::AutoClear,
                                "Library applet interactive in data event")},
          interactive_out_data_event{
              new kernel::Event(kernel::EventFlags::None,
                                "Library applet interactive out data event")} {}

    // Data

    // In
    void PushInData(IStorage* data) { in_data.PushData(data); }

    IStorage* PopInData() { return in_data.PopData(); }

    // Out
    void PushOutData(IStorage* data) { out_data.PushData(data); }

    IStorage* PopOutData() { return out_data.PopData(); }

    // Interactive in
    void PushInteractiveInData(IStorage* data) {
        interactive_in_data.PushData(data);
        interactive_in_data_event->Signal();
    }

    IStorage* PopInteractiveInData() { return interactive_in_data.PopData(); }

    // Interactive out
    void PushInteractiveOutData(IStorage* data) {
        interactive_out_data.PushData(data);
        interactive_out_data_event->Signal();
    }

    IStorage* PopInteractiveOutData() { return interactive_out_data.PopData(); }

    // Events
    kernel::Event* GetStateChangedEvent() { return state_changed_event; }

    kernel::Event* GetInteractiveInDataEvent() {
        return interactive_in_data_event;
    }

    kernel::Event* GetInteractiveOutDataEvent() {
        return interactive_out_data_event;
    }

  private:
    LibraryAppletMode mode;

    kernel::Event* state_changed_event;
    kernel::Event* interactive_in_data_event;
    kernel::Event* interactive_out_data_event;

    StorageQueue in_data;
    StorageQueue out_data;
    StorageQueue interactive_in_data;
    StorageQueue interactive_out_data;
};

} // namespace hydra::horizon::services::am
