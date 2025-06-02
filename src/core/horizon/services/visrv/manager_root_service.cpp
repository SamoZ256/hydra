#include "core/horizon/services/visrv/manager_root_service.hpp"

#include "core/horizon/services/visrv/application_display_service.hpp"

namespace hydra::horizon::services::visrv {

// TODO: is this really correct?
DEFINE_SERVICE_COMMAND_TABLE(IManagerRootService, 0, GetDisplayService, 2,
                             GetDisplayServiceWithProxyNameExchange)

result_t IManagerRootService::GetDisplayService(add_service_fn_t add_service) {
    add_service(new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

result_t IManagerRootService::GetDisplayServiceWithProxyNameExchange(
    add_service_fn_t add_service) {
    // TODO: should take input u64 and u32
    add_service(new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
