#include "core/horizon/services/aocsrv/add_on_content_manager.hpp"

namespace hydra::horizon::services::aocsrv {

DEFINE_SERVICE_COMMAND_TABLE(IAddOnContentManager, 3, ListAddOnContent)

result_t IAddOnContentManager::ListAddOnContent(
    u32 start_index, u32 buffer_size, u32* out_count,
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    LOG_FUNC_STUBBED(Services);

    // HACK
    *out_count = 0;
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::aocsrv
