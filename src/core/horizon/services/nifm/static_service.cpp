#include "core/horizon/services/nifm/static_service.hpp"

#include "core/horizon/services/nifm/general_service.hpp"

namespace hydra::horizon::services::nifm {

DEFINE_SERVICE_COMMAND_TABLE(IStaticService, 4, CreateGeneralServiceOld, 5,
                             CreateGeneralService)

result_t IStaticService::CreateGeneralServiceOld(add_service_fn_t add_service) {
    add_service(new IGeneralService());
    return RESULT_SUCCESS;
}

result_t IStaticService::CreateGeneralService(add_service_fn_t add_service) {
    // TODO: some PID stuff
    add_service(new IGeneralService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nifm
