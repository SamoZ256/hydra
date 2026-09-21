#include "core/horizon/kernel/hipc/session.hpp"

#include "core/horizon/kernel/hipc/client_session.hpp"
#include "core/horizon/kernel/hipc/cmif.hpp"
#include "core/horizon/kernel/hipc/server_session.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::kernel::hipc {

Session::Session(ServerSession* server_side_, ClientSession* client_side_,
                 std::string_view debug_name)
    : AutoObject(TYPE_ID, debug_name), server_side{server_side_},
      client_side{client_side_} {
    server_side->setParent(this);
    client_side->setParent(this);
}

void Session::onServerClose() {
    server_side = nullptr;
    if (client_side != nullptr)
        client_side->onServerClose();
}

void Session::onClientClose() {
    client_side = nullptr;
    if (server_side != nullptr)
        server_side->onClientClose();
}

} // namespace hydra::horizon::kernel::hipc
