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

class IRequest : public IService {
  public:
    IRequest();

  protected:
    result_t RequestImpl(RequestContext& context, u32 id) override;

  private:
    kernel::Event* events[2];

    // Commands
    result_t GetRequestState(RequestState* out_state);
    STUB_REQUEST_COMMAND(GetResult);
    result_t
    GetSystemEventReadableHandles(kernel::Process* process,
                                  OutHandle<HandleAttr::Copy> out_handle0,
                                  OutHandle<HandleAttr::Copy> out_handle1);
    STUB_REQUEST_COMMAND(Cancel);
    STUB_REQUEST_COMMAND(Submit);
    STUB_REQUEST_COMMAND(SetRequirementPreset);
};

} // namespace hydra::horizon::services::nifm
