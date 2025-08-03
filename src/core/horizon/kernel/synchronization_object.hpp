#pragma once

#include "core/horizon/kernel/auto_object.hpp"

namespace hydra::horizon::kernel {

class IThread;

typedef std::function<void()> signal_callback_fn_t;

class SynchronizationObject : public AutoObject {
  public:
    SynchronizationObject(
        bool signalled_ = false,
        const std::string_view debug_name = "SynchronizationObject")
        : AutoObject(debug_name), signalled{signalled_} {}

    void AddWaitingThread(IThread* thread);
    void RemoveWaitingThread(IThread* thread);
    void AddSignalCallback(signal_callback_fn_t callback);

    void Signal();
    bool Clear();

  private:
    std::mutex mutex;
    LinkedList<IThread*> waiting_threads;
    std::vector<signal_callback_fn_t> signal_callbacks;
    bool signalled{false};
};

} // namespace hydra::horizon::kernel
