#include "horizon/services/am/common_state_getter.hpp"

#include "horizon/os.hpp"
#include "horizon/services/service_base.hpp"
#include "horizon/state_manager.hpp"

namespace Hydra::Horizon::Services::Am {

namespace {

enum class OperationMode {
    Handheld,
    Console,
};

}

DEFINE_SERVICE_COMMAND_TABLE(ICommonStateGetter, 0, GetEventHandle, 1,
                             ReceiveMessage, 4, DisallowToEnterSleep, 5,
                             GetOperationMode, 6, GetPerformanceMode, 9,
                             GetCurrentFocusState)

void ICommonStateGetter::GetEventHandle(REQUEST_COMMAND_PARAMS) {
    writers.copy_handles_writer.Write(
        StateManager::GetInstance().GetMsgEvent().id);
}

void ICommonStateGetter::ReceiveMessage(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(StateManager::GetInstance().ReceiveMessage());
}

void ICommonStateGetter::GetOperationMode(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(OS::GetInstance().IsInHandheldMode()
                             ? OperationMode::Handheld
                             : OperationMode::Console);
}

void ICommonStateGetter::GetCurrentFocusState(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(StateManager::GetInstance().GetFocusState());
}

} // namespace Hydra::Horizon::Services::Am
