#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/timesrv/const.hpp"

namespace hydra::horizon::services::timesrv {

class ISteadyClock : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t GetCurrentTimePoint(RequestContext* ctx,
                                 SteadyClockTimePoint* out_time_point);
};

} // namespace hydra::horizon::services::timesrv
