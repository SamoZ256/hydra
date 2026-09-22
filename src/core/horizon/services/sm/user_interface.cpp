#include "core/horizon/services/sm/user_interface.hpp"

#include "core/horizon/kernel/hipc/client_port.hpp"
#include "core/horizon/kernel/hipc/port.hpp"
#include "core/horizon/kernel/hipc/server_port.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::sm {

DEFINE_SERVICE_COMMAND_TABLE(IUserInterface, 0, registerClient, 1,
                             getServiceHandle, 2, registerService, 65100,
                             atmosphereHasService, 65101, atmosphereWaitService)

result_t
IUserInterface::getServiceHandle(System* system, kernel::Process* process,
                                 u64 name,
                                 OutHandle<HandleAttr::Move> out_handle) {
    LOG_DEBUG(Services, "Service name: \"{}\"", u64AsString(name));

    auto client_port = system->getOs().getServiceManager().getPort(name);
    if (client_port == nullptr) {
        LOG_WARN(Services, "Unknown service name \"{}\"", u64AsString(name));
        return MAKE_RESULT(Svc, kernel::Error::NotFound); // TODO: module
    }

    // TODO: can it be domain?
    ASSERT_DEBUG(!isDomain(), Services,
                 "sm::IUserInterface cannot be a domain service");
    auto client_session = client_port->connect();
    out_handle = process->addHandleNoRetain(client_session);

    return RESULT_SUCCESS;
}

result_t
IUserInterface::registerService(System* system, kernel::Process* process,
                                u64 name, bool is_light, i32 max_sessions,
                                OutHandle<HandleAttr::Move> out_port_handle) {
    (void)is_light;
    (void)max_sessions;

    LOG_DEBUG(Services, "Service name: \"{}\"", u64AsString(name));

    // Debug
    std::string debug_name = u64AsString(name);

    // Session
    auto server_port = new kernel::hipc::ServerPort(
        fmt::format("\"{}\" server port", debug_name));
    auto client_port = new kernel::hipc::ClientPort(
        fmt::format("\"{}\" client port", debug_name));
    new kernel::hipc::Port(server_port, client_port,
                           fmt::format("\"{}\" port", debug_name));

    // Register server side
    out_port_handle = process->addHandle(server_port);

    // Register client side
    system->getOs().getServiceManager().registerPort(name, client_port);

    return RESULT_SUCCESS;
}

result_t IUserInterface::atmosphereHasService(System* system, u64 name,
                                              bool* out_has_service) {
    LOG_DEBUG(Services, "Service name: \"{}\"", u64AsString(name));

    auto client_port = system->getOs().getServiceManager().getPort(name);
    *out_has_service = (client_port != nullptr);
    return RESULT_SUCCESS;
}

result_t IUserInterface::atmosphereWaitService(u64 name) {
    // TODO: does this wait for the service to start?
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "name: {}", u64AsString(name));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::sm
