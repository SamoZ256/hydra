#include "core/horizon/services/apm/manager_privileged.hpp"

#include "core/horizon/services/apm/session.hpp"

namespace hydra::horizon::services::apm {

DEFINE_SERVICE_COMMAND_TABLE(IManagerPrivileged, 0, openSession)

result_t IManagerPrivileged::openSession(RequestContext* ctx) {
    addService(*ctx, new ISession());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::apm
