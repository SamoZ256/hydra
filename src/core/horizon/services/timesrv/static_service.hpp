#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::timesrv {

class IStaticService : public IService {
  protected:
    result_t requestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t getStandardUserSystemClock(RequestContext* ctx);
    result_t getStandardNetworkSystemClock(RequestContext* ctx);
    result_t getStandardSteadyClock(RequestContext* ctx);
    result_t getTimeZoneService(RequestContext* ctx);
    result_t getStandardLocalSystemClock(RequestContext* ctx);
    result_t getEphemeralNetworkSystemClock(RequestContext* ctx);
    result_t
    getSharedMemoryNativeHandle(System* system, kernel::Process* process,
                                OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(calculateMonotonicSystemClockBaseTimePoint);
};

} // namespace hydra::horizon::services::timesrv
