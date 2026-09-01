#include "core/horizon/kernel/synchronization_object.hpp"

#include "core/horizon/kernel/thread.hpp"

namespace hydra::horizon::kernel {

void SynchronizationObject::AddWaitingThread(IThread* thread) {
    std::scoped_lock lock(mutex);
    if (signalled) {
        thread->Resume(this);
    } else {
        ASSERT_DEBUG(waiting_threads.addFirst(thread).has_value(), Kernel,
                     "Fail to add waiting thread");
    }
}

void SynchronizationObject::RemoveWaitingThread(IThread* thread) {
    std::scoped_lock lock(mutex);
    waiting_threads.remove(thread);
}

void SynchronizationObject::AddSignalCallback(
    const signal_callback_fn_t& callback) {
    std::scoped_lock lock(mutex);
    if (signalled)
        callback();
    else
        signal_callbacks.push_back(callback);
}

void SynchronizationObject::Signal() {
    std::scoped_lock lock(mutex);
    if (signalled)
        return;

    signalled = true;

    for (auto waiting_thread = waiting_threads.getHead();
         waiting_thread.has_value();
         waiting_thread = waiting_thread.value()->getNext()) {
        waiting_thread.value()->get()->Resume(this);
    }
    waiting_threads.clear();

    for (auto& callback : signal_callbacks)
        callback();
    signal_callbacks.clear();
}

bool SynchronizationObject::Clear() {
    bool was_signalled;
    {
        std::unique_lock<std::mutex> lock(mutex);
        was_signalled = signalled;
        signalled = false;
    }

    return was_signalled;
}

} // namespace hydra::horizon::kernel
