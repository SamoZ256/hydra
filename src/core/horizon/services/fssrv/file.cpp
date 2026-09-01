#include "core/horizon/services/fssrv/file.hpp"

#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/filesystem.hpp"

namespace hydra::horizon::services::fssrv {

DEFINE_SERVICE_COMMAND_TABLE(IFile, 0, Read, 1, Write, 2, Flush, 3, SetSize, 4,
                             GetSize)

IFile::IFile(filesystem::IFile* file_, filesystem::FileOpenFlags flags)
    : file{file_}, stream{file->Open(flags)} {}

IFile::~IFile() { delete stream; }

// TODO: option
result_t IFile::Read(Aligned<u32, 8> option, u64 offset, u64 size,
                     u64* out_written_size,
                     OutBuffer<BufferAttr::MapAlias> out_buffer) {
    (void)option;

    LOG_DEBUG(Services, "Offset: 0x{:08x}, size: 0x{:08x}", offset, size);

    const auto max_size = stream->getSize() - offset;
    if (size > max_size) {
        LOG_WARN(Services, "Reading {} bytes, but maximum readable size is {}",
                 size, max_size);
        size = max_size;
    }

    stream->seekTo(offset);
    stream->readToSpan(out_buffer.stream->writeReturningSpan<u8>(size));

    *out_written_size = size;
    return RESULT_SUCCESS;
}

// TODO: option
result_t IFile::Write(Aligned<u32, 8> option, u64 offset, u64 size,
                      InBuffer<BufferAttr::MapAlias> in_buffer) {
    (void)option;

    LOG_DEBUG(Services, "Offset: 0x{:08x}, size: 0x{:08x}", offset, size);

    stream->seekTo(offset);
    stream->writeSpan(in_buffer.stream->readSpan<u8>(size));

    return RESULT_SUCCESS;
}

result_t IFile::Flush() {
    file->Flush();
    return RESULT_SUCCESS;
}

result_t IFile::SetSize(u64 size) {
    file->Resize(size);
    return RESULT_SUCCESS;
}

result_t IFile::GetSize(u64* out_size) {
    *out_size = file->GetSize();
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
