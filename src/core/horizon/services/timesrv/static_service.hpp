#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::timesrv {

class IStaticService : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t GetStandardUserSystemClock(RequestContext* ctx);
    result_t GetStandardNetworkSystemClock(RequestContext* ctx);
    result_t GetStandardSteadyClock(RequestContext* ctx);
    result_t GetTimeZoneService(RequestContext* ctx);
    result_t GetStandardLocalSystemClock(RequestContext* ctx);
    result_t GetEphemeralNetworkSystemClock(RequestContext* ctx);
    result_t
    GetSharedMemoryNativeHandle(System* system, kernel::Process* process,
                                OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(CalculateMonotonicSystemClockBaseTimePoint);
};

} // namespace hydra::horizon::services::timesrv
