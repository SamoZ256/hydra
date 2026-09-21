#include "core/horizon/services/am/application_proxy_service.hpp"

#include "core/horizon/services/am/application_proxy.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IApplicationProxyService, 0, openApplicationProxy)

result_t IApplicationProxyService::openApplicationProxy(RequestContext* ctx) {
    addService(*ctx, new IApplicationProxy());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
