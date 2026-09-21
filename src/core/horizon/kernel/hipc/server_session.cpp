#include "core/horizon/kernel/hipc/server_session.hpp"

#include "core/horizon/kernel/hipc/session.hpp"
#include "core/horizon/kernel/thread.hpp"

namespace hydra::horizon::kernel::hipc {

constexpr u64 MSG_BUFFER_MAX_SIZE = 0x2000; // TODO: what should this be?

ServerSession::~ServerSession() {
    // Resume the client thread if we still have an active request
    if (auto active_req = active_request)
        active_req->client_thread->cancelSync();

    parent->onServerClose();
}

void ServerSession::onClientClose() {
    std::scoped_lock lock(mutex);
    client_open = false;

    // Signal the server that client has closed
    signal();
}

void ServerSession::receive(IThread* crnt_thread) {
    std::scoped_lock lock(mutex);
    ASSERT_DEBUG(!requests.empty(), Kernel, "No requests");
    active_request = requests.front();
    requests.pop();

    // Copy the message to server TLS
    std::memcpy(reinterpret_cast<void*>(crnt_thread->getTlsPtr()),
                reinterpret_cast<void*>(active_request->ptr),
                MSG_BUFFER_MAX_SIZE);

    // Clear if no more requests
    if (requests.empty())
        clear();
}

void ServerSession::reply(uptr ptr) {
    std::scoped_lock lock(mutex);

    // Copy the message to client TLS
    memcpy(reinterpret_cast<void*>(active_request->client_thread->getTlsPtr()),
           reinterpret_cast<void*>(ptr), MSG_BUFFER_MAX_SIZE);

    // Resume the client thread
    active_request->client_thread->resume();

    active_request = std::nullopt;
}

void ServerSession::enqueueRequest(Process* client_process,
                                   IThread* client_thread, uptr ptr) {
    std::scoped_lock lock(mutex);
    requests.push({.client_process=client_process, .client_thread=client_thread, .ptr=ptr});

    // Signal the server to process the request
    signal();
}

} // namespace hydra::horizon::kernel::hipc
