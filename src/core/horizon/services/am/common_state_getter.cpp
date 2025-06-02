#include "core/horizon/services/am/common_state_getter.hpp"

#include "core/horizon/os.hpp"
#include "core/horizon/services/const.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(ICommonStateGetter, 0, GetEventHandle, 1,
                             ReceiveMessage, 4, DisallowToEnterSleep, 5,
                             GetOperationMode, 6, GetPerformanceMode, 9,
                             GetCurrentFocusState)

result_t
ICommonStateGetter::GetEventHandle(OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = StateManager::GetInstance().GetMsgEvent().id;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::ReceiveMessage(AppletMessage* out_message) {
    const auto msg = StateManager::GetInstance().ReceiveMessage();
    if (msg == AppletMessage::None)
        return MAKE_RESULT(Am, 0x3);
    LOG_DEBUG(Services, "MESSAGE: {}", msg);

    *out_message = msg;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::GetOperationMode(OperationMode* out_mode) {
    *out_mode = OS::GetInstance().IsInHandheldMode() ? OperationMode::Handheld
                                                     : OperationMode::Console;
    return RESULT_SUCCESS;
}

result_t ICommonStateGetter::GetCurrentFocusState(AppletFocusState* out_state) {
    *out_state = StateManager::GetInstance().GetFocusState();
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
