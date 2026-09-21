#include "core/horizon/services/nifm/static_service.hpp"

#include "core/horizon/services/nifm/general_service.hpp"

namespace hydra::horizon::services::nifm {

DEFINE_SERVICE_COMMAND_TABLE(IStaticService, 4, createGeneralServiceOld, 5,
                             createGeneralService)

result_t IStaticService::createGeneralServiceOld(RequestContext* ctx) {
    addService(*ctx, new IGeneralService());
    return RESULT_SUCCESS;
}

result_t IStaticService::createGeneralService(RequestContext* ctx) {
    // TODO: some PID stuff
    addService(*ctx, new IGeneralService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nifm
