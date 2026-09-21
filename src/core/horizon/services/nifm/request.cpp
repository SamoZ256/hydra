#include "core/horizon/services/nifm/request.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::nifm {

DEFINE_SERVICE_COMMAND_TABLE(IRequest, 0, getRequestState, 1, getResult, 2,
                             getSystemEventReadableHandles, 3, cancel, 4,
                             submit, 6, setRequirementPreset, 11,
                             setConnectionConfirmationOption)

IRequest::IRequest()
    : events{
          std::make_unique<kernel::Event>(false, "IRequest system event 0"),
          std::make_unique<kernel::Event>(false, "IRequest system event 1")} {}

result_t IRequest::getRequestState(RequestState* out_state) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    // TODO: why does Minecraft not work with RequestState::Accepted?
    *out_state = RequestState::Invalid;
    return RESULT_SUCCESS;
}

result_t IRequest::getSystemEventReadableHandles(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle0,
    OutHandle<HandleAttr::Copy> out_handle1) {
    out_handle0 = process->addHandle(events[0].get());
    out_handle1 = process->addHandle(events[1].get());
    return RESULT_SUCCESS;
}

result_t
IRequest::setConnectionConfirmationOption(ConnectionConfirmationOption option) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "option: {}", option);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nifm
