#include "core/horizon/services/am/storage_accessor.hpp"

namespace hydra::horizon::services::am {

DEFINE_SERVICE_COMMAND_TABLE(IStorageAccessor, 0, GetSize, 10, Write, 11, Read)

result_t IStorageAccessor::GetSize(i64* out_size) {
    *out_size = data.GetSize();
    return RESULT_SUCCESS;
}

result_t IStorageAccessor::Write(i64 offset,
                                 InBuffer<BufferAttr::AutoSelect> buffer) {
    ASSERT_DEBUG(offset >= 0, Services, "Offset must be >= 0");

    // TODO: correct?
    const usize size = data.GetSize() - offset;

    const auto ptr = buffer.reader->ReadPtr<u8>(size);
    memcpy(data.GetPtrU8() + offset, ptr, size);
    return RESULT_SUCCESS;
}

result_t IStorageAccessor::Read(i64 offset,
                                OutBuffer<BufferAttr::AutoSelect> out_buffer) {
    ASSERT_DEBUG(offset >= 0, Services, "Offset must be >= 0");

    // TODO: correct?
    const usize size = data.GetSize() - offset;

    out_buffer.writer->WritePtr<u8>(data.GetPtrU8() + offset, size);
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::am
