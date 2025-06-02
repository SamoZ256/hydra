#include "core/horizon/services/visrv/system_root_service.hpp"

#include "core/horizon/services/visrv/application_display_service.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(ISystemRootService, 1, GetDisplayService, 3,
                             GetDisplayServiceWithProxyNameExchange)

result_t ISystemRootService::GetDisplayService(add_service_fn_t add_service) {
    // TODO: should take input u32
    add_service(new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

result_t ISystemRootService::GetDisplayServiceWithProxyNameExchange(
    add_service_fn_t add_service) {
    add_service(new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
