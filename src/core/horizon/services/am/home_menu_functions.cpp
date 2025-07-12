#include "core/horizon/services/am/home_menu_functions.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IHomeMenuFunctions, 10, RequestToGetForeground, 21,
                             GetPopFromGeneralChannelEvent)

result_t IHomeMenuFunctions::GetPopFromGeneralChannelEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->AddHandle(pop_from_general_channel_event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
