#include "core/horizon/services/psc/pm_service.hpp"

#include "core/horizon/services/psc/pm_module.hpp"

namespace hydra::horizon::services::psc {

DEFINE_SERVICE_COMMAND_TABLE(IPmService, 0, getPmModule)

result_t IPmService::getPmModule(RequestContext* ctx) {
    addService(*ctx, new IPmModule());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::psc
