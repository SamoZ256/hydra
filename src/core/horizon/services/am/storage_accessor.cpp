#include "core/horizon/services/am/storage_accessor.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IStorageAccessor, 0, getSize, 10, write, 11, read)

result_t IStorageAccessor::getSize(i64* out_size) {
    *out_size = static_cast<i64>(data.size());
    return RESULT_SUCCESS;
}

result_t IStorageAccessor::write(i64 offset,
                                 InBuffer<BufferAttr::AutoSelect> buffer) {
    ASSERT_DEBUG(offset >= 0, Services, "Offset must be >= 0");

    // TODO: correct?
    const u64 size = data.size() - static_cast<u64>(offset);

    const auto span = buffer.stream->readSpan<u8>(size);
    std::ranges::copy(span, data.data() + offset);
    return RESULT_SUCCESS;
}

result_t IStorageAccessor::read(i64 offset,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    ASSERT_DEBUG(offset >= 0, Services, "Offset must be >= 0");

    // TODO: correct?
    const u64 size = data.size() - static_cast<u64>(offset);

    out_buffer.stream->writeSpan(
        std::span<const u8>(data.data() + offset, size));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
