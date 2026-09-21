#include "core/horizon/services/aocsrv/add_on_content_manager.hpp"

namespace hydra::horizon::services::aocsrv {

DEFINE_SERVICE_COMMAND_TABLE(IAddOnContentManager, 2, countAddOnContent, 3,
                             listAddOnContent, 50, checkAddOnContentMountStatus)

result_t IAddOnContentManager::countAddOnContent(i64* out_count) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

result_t IAddOnContentManager::listAddOnContent(
    u32 start_index, u32 buffer_size, u32* out_count,
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    (void)out_buffer;
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "start index: {}, buffer size: {:#x}",
                               start_index, buffer_size);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::aocsrv
