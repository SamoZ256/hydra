#include "core/horizon/services/am/common_state_getter.hpp"

#include "core/horizon/kernel/process.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(
    ICommonStateGetter, 0, getEventHandle, 1, receiveMessage, 4,
    disallowToEnterSleep, 5, getOperationMode, 6, getPerformanceMode, 9,
    getCurrentFocusState, 60, getDefaultDisplayResolution, 61,
    getDefaultDisplayResolutionChangeEvent, 66, setCpuBoostMode, 900,
    setRequestExitToLibraryAppletAtExecuteNextProgramEnabled)

result_t
ICommonStateGetter::getEventHandle(kernel::Process* process,
                                   OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(process->getAppletState().getMsgEvent());
    return RESULT_SUCCESS;
}

result_t
ICommonStateGetter::receiveMessage(kernel::Process* process,
                                   kernel::AppletMessage* out_message) {
    const auto msg = process->getAppletState().receiveMessage();
    if (msg == kernel::AppletMessage::None)
        return MAKE_RESULT(Am, 0x3);
    LOG_DEBUG(Services, "Message: {}", msg);

    *out_message = msg;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::getOperationMode(OperationMode* out_mode) {
    *out_mode = CONFIG_INSTANCE.getHandheldMode() ? OperationMode::Handheld
                                                  : OperationMode::Console;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::getDefaultDisplayResolution(System* system,
                                                         i32* out_width,
                                                         i32* out_height) {
    const auto res = system->getOs().getDisplayResolution();
    *out_width = static_cast<i32>(res.x());
    *out_height = static_cast<i32>(res.y());
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::getDefaultDisplayResolutionChangeEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(default_display_resolution_change_event);
    return RESULT_SUCCESS;
}

result_t
ICommonStateGetter::getCurrentFocusState(kernel::Process* process,
                                         kernel::AppletFocusState* out_state) {
    *out_state = process->getAppletState().getFocusState();
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::setCpuBoostMode(oe::CpuBoostMode mode) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "mode: {}", mode);

    // TODO: pass mode to apm::ISystemManage::SetCpuBoostMode
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
