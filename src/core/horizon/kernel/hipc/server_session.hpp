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

    explicit ServerSession(std::string_view debug_name = "Server session")
        : SynchronizationObject(TYPE_ID, false, debug_name) {}
    ~ServerSession() override;

    void onClientClose();

    bool isClientOpen() {
        std::scoped_lock lock(mutex);
        return client_open;
    }

    // Server
    void receive(IThread* crnt_thread);
    void reply(uptr ptr);

    bool hasRequests() {
        std::scoped_lock lock(mutex);
        return !requests.empty();
    }

    // HACK
    kernel::Process* getActiveRequestClientProcess() {
        std::scoped_lock lock(mutex);
        return active_request->client_process;
    }

    // Client
    void enqueueRequest(Process* client_process, IThread* client_thread,
                        uptr ptr);

  private:
    Session* parent{nullptr};

    bool client_open{true};

    std::mutex mutex;
    std::queue<SessionRequest> requests;
    std::optional<SessionRequest> active_request;

  public:
    GETTER_AND_SETTER(parent, getParent, setParent);
};

} // namespace hydra::horizon::kernel::hipc
