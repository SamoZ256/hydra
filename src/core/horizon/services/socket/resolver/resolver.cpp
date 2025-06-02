#include "core/horizon/services/socket/resolver/resolver.hpp"

namespace hydra::horizon::services::socket::Resolver {

DEFINE_SERVICE_COMMAND_TABLE(IResolver, 6, GetAddrInfoRequest)

result_t IResolver::GetAddrInfoRequest() {
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::socket::Resolver
