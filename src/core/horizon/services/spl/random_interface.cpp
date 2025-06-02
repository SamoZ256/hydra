#include "core/horizon/services/spl/random_interface.hpp"

namespace hydra::horizon::services::spl {

DEFINE_SERVICE_COMMAND_TABLE(IRandomInterface, 0, GenerateRandomBytes)

result_t IRandomInterface::GenerateRandomBytes(
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    // TODO: use a proper random number generator
    for (u32 i = 0; i < out_buffer.writer->GetSize(); i++)
        out_buffer.writer->Write<u8>(rand() % 256);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::spl
