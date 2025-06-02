#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::socket::Resolver {

class IResolver : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    // Commands
    result_t GetAddrInfoRequest();
};

} // namespace hydra::horizon::services::socket::Resolver
