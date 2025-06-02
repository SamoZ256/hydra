#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::socket {

class IClient : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t RegisterClient(u64* out_unknown);
    STUB_REQUEST_COMMAND(StartMonitoring);
    STUB_REQUEST_COMMAND(Socket);
    STUB_REQUEST_COMMAND(Connect);
    STUB_REQUEST_COMMAND(Close);
};

} // namespace hydra::horizon::services::socket
