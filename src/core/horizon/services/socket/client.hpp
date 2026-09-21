#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::socket {

class IClient : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t registerClient(u64* out_unknown);
    STUB_REQUEST_COMMAND(startMonitoring);
    STUB_REQUEST_COMMAND(socket);
    STUB_REQUEST_COMMAND(poll);
    STUB_REQUEST_COMMAND(bind);
    STUB_REQUEST_COMMAND(connect);
    STUB_REQUEST_COMMAND(listen);
    STUB_REQUEST_COMMAND(fcntl);
    STUB_REQUEST_COMMAND(setSockOpt);
    STUB_REQUEST_COMMAND(shutdown);
    STUB_REQUEST_COMMAND(close);
};

} // namespace hydra::horizon::services::socket
