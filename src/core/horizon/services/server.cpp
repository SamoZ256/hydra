#include "core/horizon/services/server.hpp"

#include "core/horizon/kernel/hipc/server_port.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services {

void Server::start() {
    // Process
    // TODO: process

    // Thread
    thread.emplace(
        nullptr, 0x20,
        [this](const kernel::should_stop_fn_t& should_stop) {
            mainLoop(should_stop);
        },
        "Service server thread");
    thread->start();
}

void Server::stop() { thread = std::nullopt; }

void Server::registerPort(kernel::hipc::ServerPort* port,
                          create_service_fn_t service_creator) {
    ports.push_back(port);
    port_service_creators.insert({port, std::move(service_creator)});
}

void Server::registerSession(kernel::hipc::ServerSession* session,
                             IService* service) {
    sessions.push_back(session);
    session_services.insert({session, service});
}

void Server::mainLoop(const kernel::should_stop_fn_t& should_stop) {
    kernel::hipc::ServerSession* reply_target_session = nullptr;
    while (true) {
        // Wait for incoming requests
        // TODO: don't create a new vector
        std::vector<kernel::SynchronizationObject*> sync_objs;
        sync_objs.reserve(ports.size() + sessions.size());
        sync_objs.insert(sync_objs.end(), ports.begin(), ports.end());
        sync_objs.insert(sync_objs.end(), sessions.begin(), sessions.end());

        u32 signalled_index;
        const auto res = system.getOs().getKernel().replyAndReceive(
            &thread.value(), sync_objs, reply_target_session,
            kernel::INFINITE_TIMEOUT, signalled_index);
        switch (res) {
        case RESULT_SUCCESS: {
            if (signalled_index < ports.size()) {
                // Incomming connection
                auto port = ports[signalled_index];
                auto session = port->acceptSession();
                registerSession(session, port_service_creators.at(port)());

                // Reset the reply target
                reply_target_session = nullptr;
            } else {
                // Incoming request
                const auto session_index =
                    static_cast<u32>(signalled_index - ports.size());
                auto session = sessions[session_index];
                auto service = session_services.at(session);

                service->handleRequest(system,
                                       session->getActiveRequestClientProcess(),
                                       thread->getTlsPtr());

                // Set the reply target
                reply_target_session = session;
            }

            break;
        }
        case MAKE_RESULT(Svc, 123): { // SessionClosed
            ASSERT_DEBUG(signalled_index >= ports.size(), Services,
                         "Invalid signalled index {}", signalled_index);
            const auto session_index =
                static_cast<u32>(signalled_index - ports.size());
            auto session = sessions[session_index];
            auto service = session_services.at(session);

            // Service
            service->release();
            session_services.erase(session);

            // Session

            // Handle all requests
            while (session->hasRequests()) {
                session->receive(&thread.value());
                service->handleRequest(system,
                                       session->getActiveRequestClientProcess(),
                                       thread->getTlsPtr());
                session->reply(thread->getTlsPtr());
            }

            // Release
            session->clear(); // TODO: is this necessary?
            session->release();
            sessions.erase(sessions.begin() + session_index);

            // Reset the reply target
            reply_target_session = nullptr;

            break;
        }
        case MAKE_RESULT(Svc, kernel::Error::Cancelled):
            return;
        default:
            LOG_FATAL(Services, "Failed to reply and receive: 0x{:08x}", res);
            break;
        }

        // Check for exit
        if (should_stop())
            break;
    }
}

} // namespace hydra::horizon::services
