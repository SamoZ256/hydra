#pragma once

#include "core/horizon/services/am/proxy.hpp"

namespace hydra::horizon::services::am {

class ILibraryAppletProxy : public IProxy {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t OpenLibraryAppletSelfAccessor(RequestContext* ctx);
};

} // namespace hydra::horizon::services::am
