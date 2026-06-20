#pragma once

#include "core/horizon/kernel/hipc/const.hpp"
#include "core/horizon/kernel/synchronization_object.hpp"

namespace hydra::horizon::kernel {
class Process;
}

namespace hydra::horizon::kernel::hipc {

class Session;

struct SessionRequest {
    Process* client_process;
    IThread* client_thread;
    uptr ptr;
};

// TODO: should maintain a reference to the parent session
class ServerSession : public SynchronizationObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::ServerSession;

    ServerSession(std::string_view debug_name = "Server session")
        : SynchronizationObject(TYPE_ID, false, debug_name) {}
    ~ServerSession() override;

    void OnClientClose();

    bool IsClientOpen() {
        std::lock_guard lock(mutex);
        return client_open;
    }

    // Server
    void Receive(IThread* crnt_thread);
    void Reply(uptr ptr);

    bool HasRequests() {
        std::lock_guard lock(mutex);
        return !requests.empty();
    }

    // HACK
    kernel::Process* GetActiveRequestClientProcess() {
        std::lock_guard lock(mutex);
        return active_request->client_process;
    }

    // Client
    void EnqueueRequest(Process* client_process, IThread* client_thread,
                        uptr ptr);

  private:
    Session* parent{nullptr};

    bool client_open{true};

    std::mutex mutex;
    std::queue<SessionRequest> requests;
    std::optional<SessionRequest> active_request;

  public:
    GETTER_AND_SETTER(parent, GetParent, SetParent);
};

} // namespace hydra::horizon::kernel::hipc
