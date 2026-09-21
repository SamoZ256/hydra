#include "core/horizon/services/socket/client.hpp"

namespace hydra::horizon::services::socket {

DEFINE_SERVICE_COMMAND_TABLE(IClient, 0, registerClient, 1, startMonitoring, 2,
                             socket, 6, poll, 13, bind, 14, connect, 18, listen,
                             20, fcntl, 21, setSockOpt, 22, shutdown, 26, close)

result_t IClient::registerClient(u64* out_unknown) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_unknown = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::socket
