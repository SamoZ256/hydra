#include "core/horizon/services/fssrv/file.hpp"

#include "core/horizon/filesystem/file_base.hpp"
#include "core/horizon/filesystem/filesystem.hpp"

namespace hydra::horizon::services::fssrv {

DEFINE_SERVICE_COMMAND_TABLE(IFile, 0, Read, 1, Write, 2, Flush, 3, SetSize, 4,
                             GetSize)

IFile::IFile(filesystem::FileBase* file_, filesystem::FileOpenFlags flags)
    : file{file_}, stream(file->Open(flags)) {}

IFile::~IFile() { file->Close(stream); }

// TODO: option
result_t IFile::Read(u32 option, u32 _pad, i64 offset, u64 size,
                     u64* out_written_size,
                     OutBuffer<BufferAttr::MapAlias> out_buffer) {
    LOG_DEBUG(Services, "Offset: 0x{:08x}, size: 0x{:08x}", offset, size);

    ASSERT_DEBUG(offset >= 0, Services, "Offset ({}) must be >= 0", offset);

    auto reader = stream.CreateReader();
    const auto max_size = reader.GetSize() - offset;
    if (size > max_size) {
        LOG_WARN(Services, "Reading {} bytes, but maximum readable size is {}",
                 size, max_size);
        size = max_size;
    }

    reader.Seek(offset);
    reader.ReadPtr(out_buffer.writer->GetBase(), size);

    *out_written_size = size;
    return RESULT_SUCCESS;
}

// TODO: option
result_t IFile::Write(u32 option, u32 _pad, i64 offset, u64 size,
                      InBuffer<BufferAttr::MapAlias> in_buffer) {
    LOG_DEBUG(Services, "Offset: 0x{:08x}, size: 0x{:08x}", offset, size);

    ASSERT_DEBUG(offset >= 0, Services, "Offset ({}) must be >= 0", offset);

    auto writer = stream.CreateWriter();
    writer.Seek(offset);
    writer.WritePtr(in_buffer.reader->GetBase(), size);

    return RESULT_SUCCESS;
}

result_t IFile::SetSize(i64 size) {
    file->Resize(size);
    return RESULT_SUCCESS;
}

result_t IFile::GetSize(i64* out_size) {
    *out_size = file->GetSize();
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
