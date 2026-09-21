#include "core/horizon/services/mii/static_service.hpp"

#include "core/horizon/services/mii/database_service.hpp"

namespace hydra::horizon::services::mii {

DEFINE_SERVICE_COMMAND_TABLE(IStaticService, 0, getDatabaseService)

result_t IStaticService::getDatabaseService(RequestContext* ctx) {
    addService(*ctx, new IDatabaseService());
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::mii
