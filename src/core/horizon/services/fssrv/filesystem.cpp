#include "core/horizon/services/fssrv/filesystem.hpp"

#include "core/horizon/filesystem/directory.hpp"
#include "core/horizon/filesystem/file.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/fssrv/directory.hpp"
#include "core/horizon/services/fssrv/file.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::fssrv {

DEFINE_SERVICE_COMMAND_TABLE(IFileSystem, 0, createFile, 1, deleteFile, 2,
                             createDirectory, 3, deleteDirectory, 4,
                             deleteDirectoryRecursively, 5, renameFile, 7,
                             getEntryType, 8, openFile, 9, openDirectory, 10,
                             commit, 11, getFreeSpaceSize, 12,
                             getTotalSpaceSize, 14, getFileTimeStampRaw)

#define READ_PATH_IMPL(path_var, debug_name)                                   \
    [[maybe_unused]] const auto path_var =                                     \
        mount +                                                                \
        std::string(                                                           \
            in_##path_var##_buffer.stream->readNullTerminatedString());        \
    LOG_DEBUG(Services, debug_name ": {}", path);
#define READ_PATH() READ_PATH_IMPL(path, "Path")

// TODO: flags
result_t
IFileSystem::createFile(System* system, CreateOption flags, u64 size,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    (void)flags;

    READ_PATH();

    // HACK
    if (size > 256_MiB) {
        LOG_WARN(Services, "File too large (size: 0x{:08x})", size);
        size = 16_MiB;
    }

    const auto res = system->getOs().getFilesystem().createFile(
        path, size, true); // TODO: should create_intermediate be true?
    if (res == filesystem::FsResult::AlreadyExists) {
        LOG_WARN(Services, "File \"{}\" already exists", path);
    } else
        ASSERT(res == filesystem::FsResult::Success, Services,
               "Failed to create file \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::deleteFile(System* system,
                        InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->getOs().getFilesystem().deleteEntry(path);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Failed to delete file \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    return RESULT_SUCCESS;
}

result_t
IFileSystem::createDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->getOs().getFilesystem().createDirectory(
        path, true); // TODO: should create_intermediate be true?
    if (res == filesystem::FsResult::AlreadyExists) {
        LOG_WARN(Services, "Directory \"{}\" already exists", path);
    } else
        ASSERT(res == filesystem::FsResult::Success, Services,
               "Failed to create directory \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::deleteDirectory(System* system,
                             InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->getOs().getFilesystem().deleteEntry(path);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to delete directory \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t IFileSystem::deleteDirectoryRecursively(
    System* system, InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    const auto res = system->getOs().getFilesystem().deleteEntry(path, true);
    ASSERT(res == filesystem::FsResult::Success, Services,
           "Failed to delete directory recursively \"{}\": {}", path, res);

    return RESULT_SUCCESS;
}

result_t
IFileSystem::renameFile(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                        InBuffer<BufferAttr::HipcPointer> in_new_path_buffer) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();
    READ_PATH_IMPL(new_path, "New path");

    return RESULT_SUCCESS;
}

result_t
IFileSystem::getEntryType(System* system,
                          InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                          EntryType* out_entry_type) {
    READ_PATH();

    filesystem::IEntry* entry;
    const auto res = system->getOs().getFilesystem().getEntry(path, entry);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error getting entry \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    *out_entry_type =
        entry->isDirectory() ? EntryType::Directory : EntryType::File;
    return RESULT_SUCCESS;
}

result_t
IFileSystem::openFile(RequestContext* ctx, System* system,
                      filesystem::FileOpenFlags flags,
                      InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    LOG_DEBUG(Services, "Flags: {}", flags);

    filesystem::IFile* file;
    const auto res = system->getOs().getFilesystem().getFile(path, file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error opening file \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    addService(*ctx, new IFile(file, flags));
    return RESULT_SUCCESS;
}

result_t
IFileSystem::openDirectory(RequestContext* ctx, System* system,
                           DirectoryFilterFlags filter_flags,
                           InBuffer<BufferAttr::HipcPointer> in_path_buffer) {
    READ_PATH();

    LOG_DEBUG(Services, "Filter flags: {}", filter_flags);

    filesystem::Directory* directory;
    const auto res =
        system->getOs().getFilesystem().getDirectory(path, directory);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Error opening directory \"{}\": {}", path, res);
        return MAKE_RESULT(Fs, 1);
    }

    addService(*ctx, new IDirectory(directory, filter_flags));
    return RESULT_SUCCESS;
}

result_t
IFileSystem::getFreeSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                              u64* out_size) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();

    // HACK
    *out_size = 64_GiB;
    return RESULT_SUCCESS;
}

result_t
IFileSystem::getTotalSpaceSize(InBuffer<BufferAttr::HipcPointer> in_path_buffer,
                               u64* out_size) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();

    // HACK
    *out_size = 64_GiB;
    return RESULT_SUCCESS;
}

result_t IFileSystem::getFileTimeStampRaw(
    System* system, InBuffer<BufferAttr::HipcPointer> in_path_buffer,
    TimeStampRaw* out_timestamp) {
    LOG_FUNC_STUBBED(Services);

    READ_PATH();

    filesystem::IFile* file;
    const auto res = system->getOs().getFilesystem().getFile(path, file);
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
