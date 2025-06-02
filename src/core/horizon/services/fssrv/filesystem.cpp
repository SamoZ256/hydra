#include "core/horizon/services/fssrv/filesystem.hpp"

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file_base.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/fssrv/directory.hpp"
#include "core/horizon/services/fssrv/file.hpp"

namespace hydra::horizon::services::fssrv {

DEFINE_SERVICE_COMMAND_TABLE(IFileSystem, 0, CreateFile, 1, DeleteFile, 2,
                             CreateDirectory, 3, DeleteDirectory, 4,
                             DeleteDirectoryRecursively, 7, GetEntryType, 8,
                             OpenFile, 9, OpenDirectory, 10, Commit)

#define READ_PATH()                                                            \
    const auto path = mount + path_buffer.reader->ReadString();                \
    LOG_DEBUG(Services, "Path: {}", path);

result_t
IFileSystem::CreateFile(CreateOption flags, u64 size,
                        InBuffer<BufferAttr::HipcPointer> path_buffer) {
    READ_PATH();

    // HACK
    if (size > 256_MiB) {
        LOG_WARN(Services, "File too large (size: 0x{:08x})", size);
        size = 16_MiB;
    }

    const auto res = FILESYSTEM_INSTANCE.CreateFile(
        path, size, true); // TODO: should create_intermediate be true?
    if (res == filesystem::FsResult::AlreadyExists)
        LOG_WARN(Services, "File \"{}\" already exists", path);
    else
        ASSERT(res == filesystem::FsResult::Success, Services,
               "Failed to create file \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::DeleteFile(InBuffer<BufferAttr::HipcPointer> path_buffer) {
    READ_PATH();

    const auto res = FILESYSTEM_INSTANCE.DeleteEntry(path);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to delete file \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::CreateDirectory(InBuffer<BufferAttr::HipcPointer> path_buffer) {
    READ_PATH();

    const auto res = FILESYSTEM_INSTANCE.CreateDirectory(
        path, true); // TODO: should create_intermediate be true?
    if (res == filesystem::FsResult::AlreadyExists)
        LOG_WARN(Services, "Directory \"{}\" already exists", path);
    else
        ASSERT(res == filesystem::FsResult::Success, Services,
               "Failed to create directory \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::DeleteDirectory(InBuffer<BufferAttr::HipcPointer> path_buffer) {
    READ_PATH();

    const auto res = FILESYSTEM_INSTANCE.DeleteEntry(path);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to delete directory \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t IFileSystem::DeleteDirectoryRecursively(
    InBuffer<BufferAttr::HipcPointer> path_buffer) {
    READ_PATH();

    const auto res = FILESYSTEM_INSTANCE.DeleteEntry(path, true);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to delete directory recursively \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::GetEntryType(InBuffer<BufferAttr::HipcPointer> path_buffer,
                          EntryType* out_entry_type) {
    READ_PATH();

    filesystem::EntryBase* entry;
    const auto res = FILESYSTEM_INSTANCE.GetEntry(path, entry);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error getting entry \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    *out_entry_type =
        entry->IsDirectory() ? EntryType::Directory : EntryType::File;
    return RESULT_SUCCESS;
}

result_t IFileSystem::OpenFile(add_service_fn_t add_service,
                               filesystem::FileOpenFlags flags,
                               InBuffer<BufferAttr::HipcPointer> path_buffer) {
    READ_PATH();

    LOG_DEBUG(Services, "Flags: {}", flags);

    filesystem::FileBase* file;
    const auto res = FILESYSTEM_INSTANCE.GetFile(path, file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error opening file \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    add_service(new IFile(file, flags));
    return RESULT_SUCCESS;
}

result_t
IFileSystem::OpenDirectory(add_service_fn_t add_service,
                           DirectoryFilterFlags filter_flags,
                           InBuffer<BufferAttr::HipcPointer> path_buffer) {
    READ_PATH();

    LOG_DEBUG(Services, "Filter flags: {}", filter_flags);

    filesystem::Directory* directory;
    const auto res = FILESYSTEM_INSTANCE.GetDirectory(path, directory);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error opening directory \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    add_service(new IDirectory(directory, filter_flags));
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
