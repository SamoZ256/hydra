#include "core/horizon/services/nifm/request.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::nifm {

DEFINE_SERVICE_COMMAND_TABLE(IRequest, 0, GetRequestState, 1, GetResult, 2,
                             GetSystemEventReadableHandles, 3, Cancel, 4,
                             Submit, 6, SetRequirementPreset, 11,
                             SetConnectionConfirmationOption)

IRequest::IRequest()
    : events{
          std::make_unique<kernel::Event>(false, "IRequest system event 0"),
          std::make_unique<kernel::Event>(false, "IRequest system event 1")} {}

result_t IRequest::GetRequestState(RequestState* out_state) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    // TODO: why does Minecraft not work with RequestState::Accepted?
    *out_state = RequestState::Invalid;
    return RESULT_SUCCESS;
}

result_t IRequest::GetSystemEventReadableHandles(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle0,
    OutHandle<HandleAttr::Copy> out_handle1) {
    out_handle0 = process->AddHandle(events[0].get());
    out_handle1 = process->AddHandle(events[1].get());
    return RESULT_SUCCESS;
}

result_t
IRequest::SetConnectionConfirmationOption(ConnectionConfirmationOption option) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "option: {}", option);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nifm
