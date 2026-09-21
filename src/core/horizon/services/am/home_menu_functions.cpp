#include "core/horizon/services/am/home_menu_functions.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IHomeMenuFunctions, 10, requestToGetForeground, 21,
                             getPopFromGeneralChannelEvent)

result_t IHomeMenuFunctions::getPopFromGeneralChannelEvent(
    kernel::Process* process, OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(pop_from_general_channel_event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
