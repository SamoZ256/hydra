#include "core/horizon/services/fssrv/filesystem.hpp"

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/fssrv/directory.hpp"
#include "core/horizon/services/fssrv/file.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::fssrv {

DEFINE_SERVICE_COMMAND_TABLE(IFileSystem, 0, CreateFile, 1, DeleteFile, 2,
                             CreateDirectory, 3, DeleteDirectory, 4,
                             DeleteDirectoryRecursively, 5, RenameFile, 7,
                             GetEntryType, 8, OpenFile, 9, OpenDirectory, 10,
                             Commit, 11, GetFreeSpaceSize, 12,
                             GetTotalSpaceSize, 14, GetFileTimeStampRaw)

#define READ_PATH_IMPL(path_var, debug_name)                                   \
    const auto path_var =                                                      \
        mount +                                                                \
        std::string(                                                           \
            in_##path_var##_buffer.stream->ReadNullTerminatedString());        \
    LOG_DEBUG(Services, debug_name ": {}", path);
#define READ_PATH() READ_PATH_IMPL(path, "Path")

// TODO: flags
result_t
IFileSystem::CreateFile(System* system, CreateOption flags, u64 size,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    (void)flags;

    READ_PATH();

    // HACK
    if (size > 256_MiB) {
        LOG_WARN(Services, "File too large (size: 0x{:08x})", size);
        size = 16_MiB;
    }

    const auto res = system->GetOS().GetFilesystem().CreateFile(
        path, size, true); // TODO: should create_intermediate be true?
    if (res == filesystem::FsResult::AlreadyExists)
        LOG_WARN(Services, "File \"{}\" already exists", path);
    else
        ASSERT(res == filesystem::FsResult::Success, Services,
               "Failed to create file \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::DeleteFile(System* system,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->GetOS().GetFilesystem().DeleteEntry(path);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Failed to delete file \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    return RESULT_SUCCESS;
}

result_t
IFileSystem::CreateDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->GetOS().GetFilesystem().CreateDirectory(
        path, true); // TODO: should create_intermediate be true?
    if (res == filesystem::FsResult::AlreadyExists)
        LOG_WARN(Services, "Directory \"{}\" already exists", path);
    else
        ASSERT(res == filesystem::FsResult::Success, Services,
               "Failed to create directory \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::DeleteDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->GetOS().GetFilesystem().DeleteEntry(path);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to delete directory \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t IFileSystem::DeleteDirectoryRecursively(
    System* system, InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->GetOS().GetFilesystem().DeleteEntry(path, true);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to delete directory recursively \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::RenameFile(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                        InBuffer<BufferAttr::HipcPointer> in_new_path_buffer) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();
    READ_PATH_IMPL(new_path, "New path");

    return RESULT_SUCCESS;
}

result_t
IFileSystem::GetEntryType(System* system,
                          InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                          EntryType* out_entry_type) {
    READ_PATH();

    filesystem::IEntry* entry;
    const auto res = system->GetOS().GetFilesystem().GetEntry(path, entry);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error getting entry \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    *out_entry_type =
        entry->IsDirectory() ? EntryType::Directory : EntryType::File;
    return RESULT_SUCCESS;
}

result_t
IFileSystem::OpenFile(RequestContext* ctx, System* system,
                      filesystem::FileOpenFlags flags,
                      InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    LOG_DEBUG(Services, "Flags: {}", flags);

    filesystem::IFile* file;
    const auto res = system->GetOS().GetFilesystem().GetFile(path, file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error opening file \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    AddService(*ctx, new IFile(file, flags));
    return RESULT_SUCCESS;
}

result_t
IFileSystem::OpenDirectory(RequestContext* ctx, System* system,
                           DirectoryFilterFlags filter_flags,
                           InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    LOG_DEBUG(Services, "Filter flags: {}", filter_flags);

    filesystem::Directory* directory;
    const auto res =
        system->GetOS().GetFilesystem().GetDirectory(path, directory);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error opening directory \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    AddService(*ctx, new IDirectory(directory, filter_flags));
    return RESULT_SUCCESS;
}

result_t
IFileSystem::GetFreeSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                              u64* out_size) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();

    // HACK
    *out_size = 64_GiB;
    return RESULT_SUCCESS;
}

result_t
IFileSystem::GetTotalSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                               u64* out_size) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();

    // HACK
    *out_size = 64_GiB;
    return RESULT_SUCCESS;
}

result_t IFileSystem::GetFileTimeStampRaw(
    System* system, InBuffer<BufferAttr::HipcPointer> in_path_buffer,
    TimeStampRaw* out_timestamp) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();

    filesystem::IFile* file;
    const auto res = system->GetOS().GetFilesystem().GetFile(path, file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error opening file \"{}\": {}", path, res);
        // TODO: set is_valid to false?
        return MAKE_RESULT(Fs, 1);
    }

    // HACK
    *out_timestamp = {
        .is_valid = true,
    };
    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
