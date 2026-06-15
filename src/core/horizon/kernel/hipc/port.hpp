#pragma once

#include "core/horizon/kernel/auto_object.hpp"

namespace hydra::horizon::kernel::hipc {

class ServerPort;
class ClientPort;

// TODO: implement same lifetime management logic as sessions
class Port : public AutoObject {
  public:
    static constexpr AutoObjectTypeId TYPE_ID = AutoObjectTypeId::Port;

    Port(ServerPort* server_side_, ClientPort* client_side_,
         const std::string_view debug_name = "Port");

  private:
    ServerPort* server_side;
    ClientPort* client_side;

  public:
    GETTER(server_side, GetServerSide);
};

} // namespace hydra::horizon::kernel::hipc
