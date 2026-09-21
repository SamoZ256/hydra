#include "core/horizon/services/nfp/user.hpp"

namespace hydra::horizon::services::nfp {

DEFINE_SERVICE_COMMAND_TABLE(IUser, 0, initialize, 2, listDevices, 19, getState,
                             23, attachAvailabilityChangeEvent)

} // namespace hydra::horizon::services::nfp
