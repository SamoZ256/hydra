#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/ssl/sf/const.hpp"

namespace hydra::horizon::services::ssl::sf {

class ISslService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t createContext(RequestContext* ctx, Aligned<SslVersion, 8> version,
                           u64 pid_placeholder);
    result_t setInterfaceVersion(SystemVersion version);
};

} // namespace hydra::horizon::services::ssl::sf
