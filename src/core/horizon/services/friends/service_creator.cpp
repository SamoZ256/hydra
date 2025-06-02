#include "core/horizon/services/friends/service_creator.hpp"

#include "core/horizon/services/friends/friend_service.hpp"

namespace hydra::horizon::services::friends {

DEFINE_SERVICE_COMMAND_TABLE(IServiceCreator, 0, CreateFriendService)

result_t IServiceCreator::CreateFriendService(add_service_fn_t add_service) {
    add_service(new IFriendService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::friends
