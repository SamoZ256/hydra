#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/ssl/sf/const.hpp"

namespace hydra::horizon::services::ssl::sf {

class ISslService : public IService {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t CreateContext(RequestContext* ctx, aligned<SslVersion, 8> version,
                           u64 pid_placeholder);
    result_t SetInterfaceVersion(SystemVersion version);
};

} // namespace hydra::horizon::services::ssl::sf
