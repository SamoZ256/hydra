#include "core/horizon/kernel/hipc/port.hpp"

#include "core/horizon/kernel/hipc/client_port.hpp"

namespace hydra::horizon::kernel::hipc {

Port::Port(ServerPort* server_side_, ClientPort* client_side_,
           std::string_view debug_name)
    : AutoObject(TYPE_ID, debug_name), server_side{server_side_},
      client_side{client_side_} {
    client_side->SetParent(this);
}

} // namespace hydra::horizon::kernel::hipc
