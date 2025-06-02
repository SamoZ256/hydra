#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IWindowController : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetAppletResourceUserId(u64* out_aruid);
    STUB_REQUEST_COMMAND(AcquireForegroundRights);
};

} // namespace hydra::horizon::services::am
