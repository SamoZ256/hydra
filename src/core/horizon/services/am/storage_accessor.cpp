#include "core/horizon/services/am/storage_accessor.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IStorageAccessor, 0, GetSize, 10, Write, 11, Read)

result_t IStorageAccessor::GetSize(i64* out_size) {
    *out_size = static_cast<i64>(data.size());
    return RESULT_SUCCESS;
}

result_t IStorageAccessor::Write(i64 offset,
                                 InBuffer<BufferAttr::AutoSelect> buffer) {
    ASSERT_DEBUG(offset >= 0, Services, "Offset must be >= 0");

    // TODO: correct?
    const u64 size = data.size() - static_cast<u64>(offset);

    const auto span = buffer.stream->ReadSpan<u8>(size);
    std::copy(span.begin(), span.end(), data.data() + offset);
    return RESULT_SUCCESS;
}

result_t IStorageAccessor::Read(i64 offset,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    ASSERT_DEBUG(offset >= 0, Services, "Offset must be >= 0");

    // TODO: correct?
    const u64 size = data.size() - static_cast<u64>(offset);

    out_buffer.stream->WriteSpan(
        std::span<const u8>(data.data() + offset, size));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
