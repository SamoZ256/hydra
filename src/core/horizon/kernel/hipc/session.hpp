#pragma once

#include "core/horizon/kernel/auto_object.hpp"

namespace hydra::horizon::kernel {
class Process;
}

namespace hydra::horizon::kernel::hipc {

class ServerSession;
class ClientSession;

class Session : public AutoObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::Session;

    Session(ServerSession* server_side_, ClientSession* client_side_,
            std::string_view debug_name = "Session");

    void OnServerClose();
    void OnClientClose();

  private:
    // TODO: this should be stack allocated, but that wouldn't really work with
    // the current AutoObject destruction logic
    ServerSession* server_side;
    ClientSession* client_side;

  public:
    GETTER(server_side, GetServerSide);
};

} // namespace hydra::horizon::kernel::hipc
