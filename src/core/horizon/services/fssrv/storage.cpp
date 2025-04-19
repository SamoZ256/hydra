#include "core/horizon/services/fssrv/storage.hpp"

#include "core/horizon/filesystem/file_base.hpp"

namespace Hydra::Horizon::Services::Fssrv {

namespace {

struct ReadWriteIn {
    i64 offset;
    u64 size;
};

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(IStorage, 0, Read, 1, Write, 3, SetSize, 4,
                             GetSize)

IStorage::IStorage(Filesystem::FileBase* file_) : file{file_} { file->Open(); }

IStorage::~IStorage() { file->Close(); }

void IStorage::Read(REQUEST_COMMAND_PARAMS) {
    const auto in = readers.reader.Read<ReadWriteIn>();

    usize size = in.size;
    ReadImpl(writers.recv_buffers_writers[0].GetBase(), in.offset, size);
}

void IStorage::Write(REQUEST_COMMAND_PARAMS) {
    const auto in = readers.reader.Read<ReadWriteIn>();

    WriteImpl(readers.send_buffers_readers[0].GetBase(), in.offset, in.size);
}

void IStorage::GetSize(REQUEST_COMMAND_PARAMS) {
    writers.writer.Write(file->GetSize());
}

void IStorage::ReadImpl(u8* ptr, i64 offset, usize& size) {
    LOG_DEBUG(HorizonServices, "Offset: 0x{:08x}, size: 0x{:08x}", offset,
              size);

    ASSERT_DEBUG(offset >= 0, HorizonServices, "Offset ({}) must be >= 0",
                 offset);

    auto reader = file->CreateReader();
    const auto max_size = reader.GetSize() - offset;
    if (size > max_size) {
        LOG_WARNING(HorizonServices,
                    "Reading {} bytes, but maximum readable size is {}", size,
                    max_size);
        size = max_size;
    }

    reader.Seek(offset);
    reader.Read(ptr, size);
}

void IStorage::WriteImpl(u8* ptr, i64 offset, usize size) {
    LOG_DEBUG(HorizonServices, "Offset: 0x{:08x}, size: 0x{:08x}", offset,
              size);

    ASSERT_DEBUG(offset >= 0, HorizonServices, "Offset ({}) must be >= 0",
                 offset);

    auto writer = file->CreateWriter();
    writer.Seek(offset);
    writer.Write(ptr, size);
}

} // namespace Hydra::Horizon::Services::Fssrv
