#include "core/horizon/services/friends/notification_service.hpp"

#include "core/horizon/kernel/event.hpp"
#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::services::friends {

DEFINE_SERVICE_COMMAND_TABLE(INotificationService, 0, getEvent, 1, clear)

INotificationService::INotificationService(uuid_t user_id_)
    : user_id{user_id_}, event{new kernel::Event(
                             false, "friends::INotificationService event")} {}

result_t
INotificationService::getEvent(kernel::Process* process,
                               OutHandle<HandleAttr::Copy> out_handle) {
    out_handle = process->addHandle(event);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::friends
