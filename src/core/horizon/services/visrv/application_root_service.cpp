#include "core/horizon/services/visrv/application_root_service.hpp"

#include "core/horizon/services/visrv/application_display_service.hpp"

namespace hydra::horizon::services::visrv {

DEFINE_SERVICE_COMMAND_TABLE(IApplicationRootService, 0, GetDisplayService, 1,
                             GetDisplayServiceWithProxyNameExchange)

result_t IApplicationRootService::GetDisplayService(RequestContext* ctx) {
    // TODO: should take input u32
    AddService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

result_t IApplicationRootService::GetDisplayServiceWithProxyNameExchange(
    RequestContext* ctx) {
    // TODO: should take input u32
    AddService(*ctx, new IApplicationDisplayService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::visrv
