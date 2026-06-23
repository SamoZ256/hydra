#include "core/horizon/kernel/synchronization_object.hpp"

#include "core/horizon/kernel/thread.hpp"

namespace hydra::horizon::kernel {

void SynchronizationObject::AddWaitingThread(IThread* thread) {
    std::scoped_lock lock(mutex);
    if (signalled)
        thread->Resume(this);
    else
        waiting_threads.AddFirst(thread);
}

void SynchronizationObject::RemoveWaitingThread(IThread* thread) {
    std::scoped_lock lock(mutex);
    waiting_threads.Remove(thread);
}

void SynchronizationObject::AddSignalCallback(const signal_callback_fn_t& callback) {
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

    for (auto waiting_thread = waiting_threads.GetHead();
         waiting_thread != nullptr; waiting_thread = waiting_thread->GetNext())
        waiting_thread->Get()->Resume(this);
    waiting_threads.Clear();

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
