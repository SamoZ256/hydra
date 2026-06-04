#pragma once

#include "core/horizon/services/am/proxy.hpp"

namespace hydra::horizon::services::am {

class ISystemAppletProxy : public IProxy {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t GetApplicationCreator(RequestContext* ctx);
};

} // namespace hydra::horizon::services::am
