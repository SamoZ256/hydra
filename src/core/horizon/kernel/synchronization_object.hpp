#pragma once

#include "core/horizon/kernel/auto_object.hpp"

namespace hydra::horizon::kernel {

class IThread;

using signal_callback_fn_t = std::function<void()>;

class SynchronizationObject : public AutoObject {
  public:
    SynchronizationObject(AutoObjectTypeId type_id, bool signalled_ = false,
                          std::string_view debug_name = "SynchronizationObject")
        : AutoObject(type_id, debug_name), signalled{signalled_} {}

    void AddWaitingThread(IThread* thread);
    void RemoveWaitingThread(IThread* thread);
    void AddSignalCallback(signal_callback_fn_t callback);

    void Signal();
    bool Clear();

  private:
    std::mutex mutex;
    DoubleLinkedList<IThread*> waiting_threads;
    std::vector<signal_callback_fn_t> signal_callbacks;
    bool signalled{false};
};

} // namespace hydra::horizon::kernel
