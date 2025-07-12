#include "core/horizon/services/am/common_state_getter.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/horizon/os.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(
    ICommonStateGetter, 0, GetEventHandle, 1, ReceiveMessage, 4,
    DisallowToEnterSleep, 5, GetOperationMode, 6, GetPerformanceMode, 9,
    GetCurrentFocusState, 60, GetDefaultDisplayResolution, 61,
    GetDefaultDisplayResolutionChangeEvent, 66, SetCpuBoostMode, 900,
    SetRequestExitToLibraryAppletAtExecuteNextProgramEnabled)

result_t
ICommonStateGetter::GetEventHandle(kernel::Process* process,
                                   OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(process->GetAppletState().GetMsgEvent());
    return RESULT_SUCCESS;
}

result_t
ICommonStateGetter::ReceiveMessage(kernel::Process* process,
                                   kernel::AppletMessage* out_message) {
    const auto msg = process->GetAppletState().ReceiveMessage();
    if (msg == kernel::AppletMessage::None)
        return MAKE_RESULT(Am, 0x3);
    LOG_DEBUG(Services, "Message: {}", msg);

    *out_message = msg;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::GetOperationMode(OperationMode* out_mode) {
    *out_mode = OS::GetInstance().IsInHandheldMode() ? OperationMode::Handheld
                                                     : OperationMode::Console;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::GetDefaultDisplayResolution(i32* out_width,
                                                         i32* out_height) {
    // TODO: don't hardcode
    *out_width = 1920;
    *out_height = 1080;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::GetDefaultDisplayResolutionChangeEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(default_display_resolution_change_event);
    return RESULT_SUCCESS;
}

result_t
ICommonStateGetter::GetCurrentFocusState(kernel::Process* process,
                                         kernel::AppletFocusState* out_state) {
    *out_state = process->GetAppletState().GetFocusState();
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::SetCpuBoostMode(oe::CpuBoostMode mode) {
    LOG_FUNC_STUBBED(Services);

    // TODO: pass mode to apm::ISystemManage::SetCpuBoostMode
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
