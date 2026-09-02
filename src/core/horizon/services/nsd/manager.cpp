#include "core/horizon/services/nsd/manager.hpp"

namespace hydra::horizon::services::nsd {

DEFINE_SERVICE_COMMAND_TABLE(IManager, 11, GetEnvironmentIdentifier)

result_t IManager::GetEnvironmentIdentifier(
    OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    // TODO: correct?
    out_buffer.stream->writeNullTerminatedString("Ip1");
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::nsd
