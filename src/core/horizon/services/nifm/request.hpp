#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::nifm {

enum class RequestState {
    Invalid = 0,
    Free = 1,
    OnHold = 2,
    Accepted = 3,
    Blocking = 4,
};

class IRequest : public ServiceBase {
  public:
    IRequest();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::HandleWithId<kernel::Event> events[2];

    // Commands
    result_t GetRequestState(RequestState* out_state);
    STUB_REQUEST_COMMAND(GetResult);
    result_t
    GetSystemEventReadableHandles(OutHandle<HandleAttr::Copy> out_handle0,
                                  OutHandle<HandleAttr::Copy> out_handle1);
    STUB_REQUEST_COMMAND(Submit);
};

} // namespace hydra::horizon::services::nifm
