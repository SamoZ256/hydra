#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

class IWindowController : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getAppletResourceUserId(kernel::Process* process, u64* out_aruid);
    STUB_REQUEST_COMMAND(acquireForegroundRights);
};

} // namespace hydra::horizon::services::am
