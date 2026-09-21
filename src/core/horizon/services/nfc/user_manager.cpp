#include "core/horizon/services/nfc/user_manager.hpp"

#include "core/horizon/services/nfc/user.hpp"

namespace hydra::horizon::services::nfc {

DEFINE_SERVICE_COMMAND_TABLE(IUserManager, 0, createUserInterface)

result_t IUserManager::createUserInterface(RequestContext* ctx) {
    addService(*ctx, new IUser());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nfc
