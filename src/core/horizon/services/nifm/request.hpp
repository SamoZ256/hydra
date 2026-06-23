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

enum class ConnectionConfirmationOption : u32 {
    Invalid = 0,
    Prohibited = 1,
    NotRequired = 2,
    Preferred = 3,
    Required = 4,
    Forced = 5,
};

class IRequest : public IService {
  public:
    IRequest();

  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    std::array<std::unique_ptr<kernel::Event>, 2> events;

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
    result_t
    SetConnectionConfirmationOption(ConnectionConfirmationOption option);
};

} // namespace hydra::horizon::services::nifm

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::nifm::ConnectionConfirmationOption, Invalid,
    "invalid", Prohibited, "prohibited", NotRequired, "not_required", Preferred,
    "preferred", Required, "required", Forced, "forced")
