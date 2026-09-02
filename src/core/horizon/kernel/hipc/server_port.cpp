#include "core/horizon/kernel/hipc/server_port.hpp"

namespace hydra::horizon::kernel::hipc {

ServerSession* ServerPort::AcceptSession() {
    std::scoped_lock lock(mutex);
    ASSERT_DEBUG(!incomming_sessions.empty(), Kernel, "No incomming sessions");
    const auto session = incomming_sessions.front();
    incomming_sessions.pop();

    // Clear if no more incoming sessions
    if (incomming_sessions.empty())
        Clear();

    return session;
}

void ServerPort::ConnectSession(ServerSession* session) {
    std::scoped_lock lock(mutex);
    incomming_sessions.push(session);

    // Signal incomming session
    Signal();
}

} // namespace hydra::horizon::kernel::hipc
