#include "core/horizon/services/fssrv/storage.hpp"

#include "core/horizon/filesystem/file.hpp"

namespace hydra::horizon::services::fssrv {

DEFINE_SERVICE_COMMAND_TABLE(IStorage, 0, Read, 1, Write, 2, Flush, 3, SetSize,
                             4, GetSize)

IStorage::IStorage(filesystem::IFile* file_, filesystem::FileOpenFlags flags)
    : file{file_}, stream(file->Open(flags)) {}

IStorage::~IStorage() { delete stream; }

result_t IStorage::Read(u64 offset, u64 size,
                        OutBuffer<BufferAttr::MapAlias> out_buffer) {
    LOG_DEBUG(Services, "Offset: 0x{:08x}, size: 0x{:08x}", offset, size);

    const auto max_size = stream->getSize() - offset;
    if (size > max_size) {
        LOG_WARN(Services, "Reading {} bytes, but maximum readable size is {}",
                 size, max_size);
        size = max_size;
    }

    // HACK: Celeste reads into a null buffer
    if (!out_buffer.stream)
        return RESULT_SUCCESS;

    stream->seekTo(offset);
    stream->readToSpan(out_buffer.stream->writeReturningSpan<u8>(size));

    return RESULT_SUCCESS;
}

result_t IStorage::Write(u64 offset, u64 size,
                         InBuffer<BufferAttr::MapAlias> in_buffer) {
    LOG_DEBUG(Services, "Offset: 0x{:08x}, size: 0x{:08x}", offset, size);

    stream->seekTo(offset);
    stream->writeSpan(in_buffer.stream->readSpan<u8>(size));

    return RESULT_SUCCESS;
}

result_t IStorage::SetSize(u64 size) {
    file->Resize(size);
    return RESULT_SUCCESS;
}

result_t IStorage::GetSize(u64* out_size) {
    *out_size = file->GetSize();
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
