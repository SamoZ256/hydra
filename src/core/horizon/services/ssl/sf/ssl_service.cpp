#include "core/horizon/services/ssl/sf/ssl_service.hpp"

#include "core/horizon/services/ssl/sf/ssl_context.hpp"

namespace hydra::horizon::services::ssl::sf {

DEFINE_SERVICE_COMMAND_TABLE(ISslService, 0, CreateContext, 5,
                             SetInterfaceVersion)

result_t ISslService::CreateContext(RequestContext* ctx,
                                    Aligned<SslVersion, 8> version,
                                    u64 pid_placeholder) {
    (void)pid_placeholder;

    LOG_DEBUG(Services, "Version: {}", version);

    AddService(*ctx, new ISslContext(version));
    return RESULT_SUCCESS;
}

result_t ISslService::SetInterfaceVersion(SystemVersion version) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "version: {}", version);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::ssl::sf
