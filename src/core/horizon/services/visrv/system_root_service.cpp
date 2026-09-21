#include "core/horizon/services/visrv/system_root_service.hpp"

#include "core/horizon/services/visrv/application_display_service.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(ISystemRootService, 1, getDisplayService, 3,
                             getDisplayServiceWithProxyNameExchange)

result_t ISystemRootService::getDisplayService(RequestContext* ctx) {
    // TODO: should take input u32
    addService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

result_t ISystemRootService::getDisplayServiceWithProxyNameExchange(
    RequestContext* ctx) {
    addService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
