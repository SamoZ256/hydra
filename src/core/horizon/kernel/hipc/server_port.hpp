#pragma once

#include "core/horizon/kernel/synchronization_object.hpp"

namespace hydra::horizon::kernel::hipc {

class ServerSession;

class ServerPort : public SynchronizationObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::ServerPort;

    ServerPort(std::string_view debug_name = "Server port")
        : SynchronizationObject(TYPE_ID, false, debug_name) {}

    // Server
    ServerSession* AcceptSession();

    // Client
    void ConnectSession(ServerSession* session);

  private:
    std::mutex mutex;
    std::queue<ServerSession*> incomming_sessions;
};

} // namespace hydra::horizon::kernel::hipc
