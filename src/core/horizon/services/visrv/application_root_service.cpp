#include "core/horizon/services/visrv/application_root_service.hpp"

#include "core/horizon/services/visrv/application_display_service.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(IApplicationRootService, 0, getDisplayService, 1,
                             getDisplayServiceWithProxyNameExchange)

result_t IApplicationRootService::getDisplayService(RequestContext* ctx) {
    // TODO: should take input u32
    addService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

result_t IApplicationRootService::getDisplayServiceWithProxyNameExchange(
    RequestContext* ctx) {
    // TODO: should take input u32
    addService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
