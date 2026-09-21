#include "core/horizon/services/pdm/query_service.hpp"

namespace hydra::horizon::services::pdm {

DEFINE_SERVICE_COMMAND_TABLE(IQueryService, 8, queryPlayEvent, 11,
                             queryAccountPlayEvent)

result_t
IQueryService::queryPlayEvent(i32 entry_index, i32* out_total_entries,
                              OutBuffer<BufferAttr::MapAlias> out_buffer) {
    (void)out_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "entry index: {}", entry_index);

    // HACK
    *out_total_entries = 0;
    return RESULT_SUCCESS;
}

result_t IQueryService::queryAccountPlayEvent(
    i32 entry_index, uuid_t user_id, i32* out_total_entries,
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    (void)out_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "entry index: {}, user ID: {}",
                               entry_index, user_id);

    // HACK
    *out_total_entries = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::pdm
