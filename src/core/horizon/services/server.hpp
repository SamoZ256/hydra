#pragma once

#include "core/horizon/kernel/hipc/server_session.hpp"
#include "core/horizon/kernel/hipc/session.hpp"
#include "core/horizon/kernel/host_thread.hpp"
#include "core/horizon/services/service.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::kernel::hipc {
class ServerPort;
class ServerSession;
} // namespace hydra::horizon::kernel::hipc

namespace hydra::horizon::services {

using create_service_fn_t = std::function<IService*()>;

class Server {
  public:
    Server(System& system_) : system{system_} {}
    ~Server() { Stop(); }

    MAKE_NON_COPYABLE(Server);
    MAKE_NON_MOVABLE(Server);

    void Start();
    void Stop();

    void RegisterPort(kernel::hipc::ServerPort* port,
                      create_service_fn_t service_creator);
    void RegisterSession(kernel::hipc::ServerSession* session,
                         IService* service);

    IService* GetServiceForSession(kernel::hipc::ServerSession* session) {
        return session_services.at(session);
    }

  private:
    System& system;

    std::optional<kernel::HostThread> thread;

    std::map<kernel::hipc::ServerPort*, create_service_fn_t>
        port_service_creators;
    std::map<kernel::hipc::ServerSession*, IService*> session_services;

    std::vector<kernel::hipc::ServerPort*> ports;
    std::vector<kernel::hipc::ServerSession*> sessions;

    void MainLoop(const kernel::should_stop_fn_t& should_stop);
};

} // namespace hydra::horizon::services
