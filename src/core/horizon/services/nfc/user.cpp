#include "core/horizon/services/nfc/user.hpp"

namespace hydra::horizon::services::nfc {

DEFINE_SERVICE_COMMAND_TABLE(IUser, 0, initialize, 2, getState, 400, initialize,
                             402, getState)

} // namespace hydra::horizon::services::nfc
