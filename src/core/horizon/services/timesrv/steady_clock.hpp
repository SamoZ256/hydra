#pragma once

#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::timesrv {

class ISteadyClock : public ServiceBase {
  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
};

} // namespace hydra::horizon::services::timesrv
