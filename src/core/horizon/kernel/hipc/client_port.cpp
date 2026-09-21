#include "core/horizon/kernel/hipc/client_port.hpp"

#include "core/horizon/kernel/hipc/client_session.hpp"
#include "core/horizon/kernel/hipc/port.hpp"
#include "core/horizon/kernel/hipc/server_port.hpp"
#include "core/horizon/kernel/hipc/server_session.hpp"
#include "core/horizon/kernel/hipc/session.hpp"

namespace hydra::horizon::kernel::hipc {

ClientSession* ClientPort::connect() {
    auto server_session = new hipc::ServerSession();
    auto client_session = new hipc::ClientSession();
    // TODO: is it fine to just instantiate it like this?
    new hipc::Session(server_session, client_session);

    // Connect to server
    parent->getServerSide()->connectSession(server_session);

    return client_session;
}

} // namespace hydra::horizon::kernel::hipc
