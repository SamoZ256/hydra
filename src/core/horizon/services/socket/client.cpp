#include "core/horizon/services/socket/client.hpp"

namespace hydra::horizon::services::socket {

DEFINE_SERVICE_COMMAND_TABLE(IClient, 0, RegisterClient, 1, StartMonitoring, 2,
                             Socket, 14, Connect, 26, Close)

result_t IClient::RegisterClient(u64* out_unknown) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_unknown = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::socket
