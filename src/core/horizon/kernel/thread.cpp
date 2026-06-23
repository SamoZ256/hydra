#include "core/horizon/kernel/thread.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::kernel {

IThread::~IThread() noexcept {
    if (!IsStoppingOrStopped()) {
        Stop();
    }
}

void IThread::Start() {
    thread = std::jthread([&]() {
        tls_current_thread = this;

        GET_CURRENT_PROCESS_DEBUGGER().RegisterThisThread(GetDebugName());

        // TODO: don't allow null processes
        if (process)
            process->RegisterThread(this);
        Run();
        if (process)
            process->UnregisterThread(this);

        // Signal exit
        state = ThreadState::Stopped;
        Signal();

        GET_CURRENT_PROCESS_DEBUGGER().UnregisterThisThread();
    });
}

bool IThread::ProcessMessages(i64 pause_timeout_ns) {
    if (state == ThreadState::Stopping)
        return true;

    const auto timeout_time = std::chrono::steady_clock::now() +
                              std::chrono::nanoseconds(pause_timeout_ns);

    std::unique_lock<std::mutex> lock(msg_mutex);
    while (!msg_queue.empty()) {
        if (!ProcessMessagesImpl())
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

void IThread::SendMessage(ThreadMessage msg) {
    std::scoped_lock lock(msg_mutex);
    msg_queue.push(msg);
    msg_cv.notify_all(); // TODO: notify one?
}

bool IThread::ProcessMessagesImpl() {
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

void IThread::AddMutexWaiter(IThread* waiter) {
    std::scoped_lock lock(mutex_wait_mutex);
    mutex_wait_list.AddLast(waiter);
}

void IThread::RemoveMutexWaiter(IThread* waiter) {
    std::scoped_lock lock(mutex_wait_mutex);
    mutex_wait_list.Remove(waiter);
}

IThread* IThread::RelinquishMutex(uptr mutex_addr, u32& out_waiter_count) {
    std::scoped_lock lock(mutex_wait_mutex);

    // Find a new owner
    IThread* new_owner = nullptr;
    out_waiter_count = 0;
    for (auto waiter_node = mutex_wait_list.GetHead();
         waiter_node != nullptr;) {
        auto waiter = waiter_node->Get();
        if (waiter->mutex_wait_addr != mutex_addr) {
            waiter_node = waiter_node->GetNext();
            continue;
        }

        waiter_node = mutex_wait_list.Remove(waiter_node);
        if (new_owner != nullptr) {
            new_owner->AddMutexWaiter(waiter);
            out_waiter_count++;
        } else {
            new_owner = waiter;
            new_owner->mutex_wait_addr = 0x0;
        }
    }

    return new_owner;
}

IThread* GetMutexOwner(Process* process, u32 mutex) {
    return process->GetHandle<IThread>(mutex & ~MUTEX_WAIT_MASK);
}

} // namespace hydra::horizon::kernel
