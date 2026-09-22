#include "core/horizon/kernel/thread.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::kernel {

IThread::~IThread() noexcept {
    if (!isStoppingOrStopped()) {
        stop();
    }
}

void IThread::start() {
    thread = std::jthread([&] {
        tls_current_thread = this;

        GET_CURRENT_PROCESS_DEBUGGER().registerThisThread(getDebugName());

        // TODO: don't allow null processes
        if (process)
            process->registerThread(this);
        run();
        if (process)
            process->unregisterThread(this);

        // Signal exit
        state = ThreadState::Stopped;
        signal();

        GET_CURRENT_PROCESS_DEBUGGER().unregisterThisThread();
    });
}

bool IThread::processMessages(i64 pause_timeout_ns) {
    if (state == ThreadState::Stopping)
        return true;

    const auto timeout_time = std::chrono::steady_clock::now() +
                              std::chrono::nanoseconds(pause_timeout_ns);

    std::unique_lock<std::mutex> lock(msg_mutex);
    while (!msg_queue.empty()) {
        if (!processMessagesImpl())
            return true;

        if (state != ThreadState::Paused)
            return true;

        if (supervisor_pause || pause_timeout_ns == INFINITE_TIMEOUT) {
            msg_cv.wait(lock);
        } else {
            msg_cv.wait_until(lock, timeout_time);
            const auto crnt_time = std::chrono::steady_clock::now();
            if (crnt_time >= timeout_time)
                return false;
        }
    }

    return true;
}

void IThread::sendMessage(ThreadMessage msg) {
    std::scoped_lock lock(msg_mutex);
    msg_queue.push(msg);
    msg_cv.notify_all(); // TODO: notify one?
}

bool IThread::processMessagesImpl() {
    while (!msg_queue.empty()) {
        auto msg = msg_queue.front();
        msg_queue.pop();

        // Process the message
        switch (msg.type) {
        case ThreadMessageType::Stop:
            state = ThreadState::Stopping;
            return false;
        case ThreadMessageType::Pause: {
            state = ThreadState::Paused;
            if (msg.supervisor) {
                supervisor_pause = true;
            } else {
                guest_pause = true;
                sync_info = std::nullopt;
            }
            break;
        }
        case ThreadMessageType::Resume: {
            const auto& payload = msg.payload.resume;

            if (msg.supervisor)
                supervisor_pause = false;
            else
                guest_pause = false;
            if (!supervisor_pause && !guest_pause)
                state = ThreadState::Running;

            if (!msg.supervisor && !sync_info)
                sync_info = payload;
            break;
        }
        }
    }

    return true;
}

void IThread::addMutexWaiter(IThread* waiter) {
    std::scoped_lock lock(mutex_wait_mutex);
    ASSERT_DEBUG(mutex_wait_list.addLast(waiter).has_value(), Kernel,
                 "Failed to add mutex waiter");
}

void IThread::removeMutexWaiter(IThread* waiter) {
    std::scoped_lock lock(mutex_wait_mutex);
    mutex_wait_list.remove(waiter);
}

IThread* IThread::relinquishMutex(uptr mutex_addr, u32& out_waiter_count) {
    std::scoped_lock lock(mutex_wait_mutex);

    // Find a new owner
    IThread* new_owner = nullptr;
    out_waiter_count = 0;
    for (auto waiter_node = mutex_wait_list.getHead();
         waiter_node.has_value();) {
        const auto waiter_node_ = waiter_node.value();
        auto waiter = waiter_node_->get();
        if (waiter->mutex_wait_addr != mutex_addr) {
            waiter_node = waiter_node_->getNext();
            continue;
        }

        waiter_node = mutex_wait_list.remove(waiter_node_);
        if (new_owner != nullptr) {
            new_owner->addMutexWaiter(waiter);
            out_waiter_count++;
        } else {
            new_owner = waiter;
            new_owner->mutex_wait_addr = 0x0;
        }
    }

    return new_owner;
}

std::optional<IThread*> getMutexOwner(Process* process, u32 mutex) {
    // HACK
    const auto thread = process->getHandle<IThread>(mutex & ~MUTEX_WAIT_MASK);
    return (thread != nullptr ? std::make_optional(thread) : std::nullopt);
}

std::optional<IThread*> getMutexOwner(Process* process, u32* mutex_ptr) {
    if (mutex_ptr == nullptr)
        return std::nullopt;
    return getMutexOwner(process, atomicLoad(mutex_ptr));
}

} // namespace hydra::horizon::kernel
