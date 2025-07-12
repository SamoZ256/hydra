#include "core/horizon/services/nifm/request.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::nifm {

DEFINE_SERVICE_COMMAND_TABLE(IRequest, 0, GetRequestState, 1, GetResult, 2,
                             GetSystemEventReadableHandles, 3, Cancel, 4,
                             Submit)

// TODO: how come is the autoclear of the second one user-specified?
IRequest::IRequest()
    : events{new kernel::Event(kernel::EventFlags::AutoClear,
                               "IRequest system event 0"),
             new kernel::Event(kernel::EventFlags::AutoClear,
                               "IRequest system event 1")} {}

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
    out_handle0 = process->AddHandle(events[0]);
    out_handle1 = process->AddHandle(events[1]);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nifm
