#include "core/horizon/kernel/hipc/server_port.hpp"

namespace hydra::horizon::kernel::hipc {

ServerSession* ServerPort::acceptSession() {
    std::scoped_lock lock(mutex);
    ASSERT_DEBUG(!incomming_sessions.empty(), Kernel, "No incomming sessions");
    const auto session = incomming_sessions.front();
    incomming_sessions.pop();

    // Clear if no more incoming sessions
    if (incomming_sessions.empty())
        clear();

    return session;
}

void ServerPort::connectSession(ServerSession* session) {
    std::scoped_lock lock(mutex);
    incomming_sessions.push(session);

    // Signal incomming session
    signal();
}

} // namespace hydra::horizon::kernel::hipc
