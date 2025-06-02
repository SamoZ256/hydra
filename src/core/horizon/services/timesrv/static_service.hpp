#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::timesrv {

class IStaticService : public ServiceBase {
  public:
    IStaticService();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::HandleWithId<kernel::SharedMemory> shared_memory_handle;

    // Commands
    result_t GetStandardUserSystemClock(add_service_fn_t add_service);
    result_t GetStandardNetworkSystemClock(add_service_fn_t add_service);
    result_t GetStandardSteadyClock(add_service_fn_t add_service);
    result_t GetTimeZoneService(add_service_fn_t add_service);
    result_t GetStandardLocalSystemClock(add_service_fn_t add_service);
    result_t GetEphemeralNetworkSystemClock(add_service_fn_t add_service);
    result_t
    GetSharedMemoryNativeHandle(OutHandle<HandleAttr::Copy> out_handle);
    STUB_REQUEST_COMMAND(CalculateMonotonicSystemClockBaseTimePoint);
};

} // namespace hydra::horizon::services::timesrv
