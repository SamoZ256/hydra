#include "core/horizon/services/visrv/manager_root_service.hpp"

#include "core/horizon/services/visrv/application_display_service.hpp"

namespace hydra::horizon::services::visrv {

// TODO: is this really correct?
DEFINE_SERVICE_COMMAND_TABLE(IManagerRootService, 0, getDisplayService, 2,
                             getDisplayServiceWithProxyNameExchange)

result_t IManagerRootService::getDisplayService(RequestContext* ctx) {
    addService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

result_t IManagerRootService::getDisplayServiceWithProxyNameExchange(
    RequestContext* ctx) {
    // TODO: should take input u64 and u32
    addService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
