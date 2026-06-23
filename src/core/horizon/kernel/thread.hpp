#pragma once

#include "core/horizon/kernel/synchronization_object.hpp"

namespace hydra::horizon::kernel {

class Process;

constexpr u64 TLS_SIZE = 0x20000; // TODO: what should this be?
constexpr u32 MUTEX_WAIT_MASK = 0x40000000;

enum class ThreadState {
    Created,
    Running,
    Stopping,
    Stopped,
    Paused,
};

enum class ThreadMessageType {
    Stop,
    Pause,
    Resume,
};

struct ThreadSyncInfo {
    bool signalled;
    SynchronizationObject* signalled_obj;
};

struct ThreadMessage {
    ThreadMessageType type;
    bool supervisor;
    union {
        // Resume
        ThreadSyncInfo resume;
    } payload;
};

class IThread : public SynchronizationObject {
    friend class Kernel;

  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::Thread;

    IThread(Process* process_, i32 priority_,
            std::string_view debug_name = "Thread") noexcept
        : SynchronizationObject(TYPE_ID, false, debug_name), process{process_},
          priority{priority_} {}
    ~IThread() noexcept override;

    void Start();

    // Messages
    void Stop() {
        SendMessage({.type = ThreadMessageType::Stop, .supervisor = false});
    }
    // TODO: SupervisorStop?
    void Pause() {
        SendMessage({.type = ThreadMessageType::Pause, .supervisor = false});
    }
    void SupervisorPause() {
        SendMessage({.type = ThreadMessageType::Pause, .supervisor = true});
    }
    void Resume(SynchronizationObject* signalled_obj = nullptr) {
        SendMessage({.type = ThreadMessageType::Resume,
                     .supervisor = false,
                     .payload = {.resume = {.signalled = true,
                                            .signalled_obj = signalled_obj}}});
    }
    void CancelSync() {
        SendMessage({.type = ThreadMessageType::Resume,
                     .supervisor = false,
                     .payload = {.resume = {.signalled = false}}});
    }
    void SupervisorResume() {
        SendMessage({.type = ThreadMessageType::Resume, .supervisor = true});
    }

    // Must not be called from a different thread
    bool ProcessMessages(i64 pause_timeout_ns = INFINITE_TIMEOUT);
    bool WasSignalled() const {
        ASSERT_DEBUG(sync_info, Kernel, "No signal info present");
        const auto& sync_info_value = sync_info.value();
        ASSERT_DEBUG(!sync_info_value.signalled_obj, Kernel,
                     "Unexpected signalled object {}",
                     sync_info_value.signalled_obj->GetDebugName());
        return sync_info_value.signalled;
    }
    bool ConsumeSignalledObject(SynchronizationObject*& out_obj) const {
        ASSERT_DEBUG(sync_info, Kernel, "No signal info present");
        const auto& sync_info_value = sync_info.value();
        if (!sync_info_value.signalled)
            return false;

        ASSERT_DEBUG(sync_info_value.signalled_obj, Kernel,
                     "Expected signalled object (self: {})", GetDebugName());
        out_obj = sync_info_value.signalled_obj;
        return true;
    }

    bool IsStoppingOrStopped() const {
        return state == ThreadState::Stopping || state == ThreadState::Stopped;
    }

    virtual uptr GetTlsPtr() const = 0;

  protected:
    Process* process;

    virtual void Run() = 0;

    // Helpers
    void Reset() {
        state = ThreadState::Created;
        msg_queue = {};
        mutex_wait_addr = 0x0;
        cond_var_wait_addr = 0x0;
        cond_var_wait_addr = 0x0;
        mutex_wait_list.Clear();
        supervisor_pause = false;
        guest_pause = false;
    }

  private:
    i32 priority;

    ThreadState state{ThreadState::Created}; // TODO: atomic?

    std::mutex msg_mutex;
    std::condition_variable msg_cv;
    std::queue<ThreadMessage> msg_queue;

    // Mutex and cond var
    uptr mutex_wait_addr{0x0};
    u32 self_handle_for_mutex{0x0};
    uptr cond_var_wait_addr{0x0};
    std::mutex mutex_wait_mutex;
    DoubleLinkedList<IThread*> mutex_wait_list;

    // Synchronization
    bool supervisor_pause{false};
    bool guest_pause{false};
    std::optional<ThreadSyncInfo> sync_info{std::nullopt};

    std::jthread thread;

    // Helpers

    // Messages
    void SendMessage(ThreadMessage msg);
    bool ProcessMessagesImpl();

    // Mutex
    void AddMutexWaiter(IThread* waiter);
    void RemoveMutexWaiter(IThread* waiter);
    IThread* RelinquishMutex(uptr mutex_addr, u32& out_waiter_count);

  public:
    GETTER(process, GetProcess);
    GETTER(state, GetState);
};

inline thread_local IThread* tls_current_thread = nullptr;

IThread* GetMutexOwner(Process* process, u32 mutex);
inline IThread* GetMutexOwner(Process* process, u32* mutex_ptr) {
    return GetMutexOwner(process, atomic_load(mutex_ptr));
}

} // namespace hydra::horizon::kernel
