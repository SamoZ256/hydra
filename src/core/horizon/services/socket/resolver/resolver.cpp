#include "core/horizon/services/socket/resolver/resolver.hpp"

namespace hydra::horizon::services::socket::resolver {

DEFINE_SERVICE_COMMAND_TABLE(IResolver, 6, getAddrInfoRequest)

result_t IResolver::getAddrInfoRequest() {
    LOG_FUNC_NOT_IMPLEMENTED(Services);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::socket::resolver
