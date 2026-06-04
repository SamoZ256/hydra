#include "core/horizon/services/mii/image_database_service.hpp"

namespace hydra::horizon::services::mii {

DEFINE_SERVICE_COMMAND_TABLE(IImageDatabaseService, 0, Initialize, 11, GetCount)

result_t IImageDatabaseService::Initialize(bool use_database,
                                           bool* out_is_dirty) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "use database: {}", use_database);

    // HACK
    *out_is_dirty = false;
    return RESULT_SUCCESS;
}

result_t IImageDatabaseService::GetCount(u32* out_count) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::mii
