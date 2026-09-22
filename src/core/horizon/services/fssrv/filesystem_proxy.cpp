#include "core/horizon/services/fssrv/filesystem_proxy.hpp"

#include "core/horizon/filesystem/content_archive.hpp"
#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/horizon/services/fssrv/file.hpp"
#include "core/horizon/services/fssrv/filesystem.hpp"
#include "core/horizon/services/fssrv/save_data_info_reader.hpp"
#include "core/horizon/services/fssrv/storage.hpp"
#include "core/system.hpp"

namespace hydra::horizon::services::fssrv {

namespace {

std::string getSaveDataMount(kernel::Process* process,
                             const SaveDataAttribute& attr) {
    u64 title_id = attr.title_id;
    // TODO: is this correct?
    if (title_id == 0x0)
        title_id = process->getTitleId();

    switch (attr.type) {
    case SaveDataType::Account:
        return FS_USER_SAVE_DATA_PATH(title_id, attr.user_id);
    case SaveDataType::Device:
        return FS_SHARED_SAVE_DATA_PATH(title_id);
    case SaveDataType::Cache:
        return FS_CACHE_MOUNT;
    default:
        LOG_NOT_IMPLEMENTED(Services, "Save data type {}", attr.type);
        return "INVALID";
    }
}

} // namespace

DEFINE_SERVICE_COMMAND_TABLE(
    IFileSystemProxy, 0, openFileSystem, 1, setCurrentProcess, 8,
    openFileSystemWithIdObsolete, 11, openBisFileSystem, 18,
    openSdCardFileSystem, 22, createSaveDataFileSystem, 51,
    openSaveDataFileSystem, 52,
    readSaveDataFileSystemExtraDataBySaveDataSpaceId, 53,
    openReadOnlySaveDataFileSystem, 61, openSaveDataInfoReaderBySaveDataSpaceId,
    200, openDataStorageByCurrentProcess, 201, openDataStorageByProgramId, 202,
    openDataStorageByDataId, 203, openPatchDataStorageByCurrentProcess, 1003,
    disableAutoSaveDataCreation, 1005, getGlobalAccessLogMode)

// TODO: type
result_t IFileSystemProxy::openFileSystem(
    RequestContext* ctx, FileSystemProxyType type,
    InBuffer<BufferAttr::HipcPointer> path_buffer) {
    (void)type;

    // TODO: correct?
    const auto mount = path_buffer.stream->readNullTerminatedString();
    LOG_DEBUG(Services, "Mount: {}", mount);

    addService(*ctx, new IFileSystem(mount));

    return RESULT_SUCCESS;
}

// TODO: type
result_t IFileSystemProxy::openFileSystemWithIdObsolete(
    RequestContext* ctx, FileSystemProxyType type, u64 program_id,
    InBuffer<BufferAttr::HipcPointer> path_buffer) {
    (void)type;
    (void)program_id;

    // TODO: correct?
    const auto mount = path_buffer.stream->readNullTerminatedString();
    LOG_DEBUG(Services, "Mount: {}", mount);

    addService(*ctx, new IFileSystem(mount));

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::openBisFileSystem(
    BisPartitionId partition_id,
    InBuffer<BufferAttr::HipcPointer> unknown_buffer) {
    const auto unknown =
        unknown_buffer.stream
            ->readNullTerminatedString(); // TODO: what is this for?

    LOG_FUNC_WITH_ARGS_STUBBED(Services, "partition ID: {}, unknown: {}",
                               partition_id, unknown);

    return MAKE_RESULT(Fs, 1771);
}

result_t IFileSystemProxy::openSdCardFileSystem(RequestContext* ctx) {
    // TODO: correct?
    addService(*ctx, new IFileSystem(FS_SD_MOUNT));
    return RESULT_SUCCESS;
}

// TODO: creation and meta info
result_t IFileSystemProxy::createSaveDataFileSystem(
    System* system, kernel::Process* process, SaveDataAttribute attr,
    SaveDataCreationInfo creation_info, SaveDataMetaInfo meta_info) {
    (void)creation_info;
    (void)meta_info;

    std::string mount = getSaveDataMount(process, attr);
    const auto res =
        system->getOs().getFilesystem().createDirectory(mount, true);
    ASSERT(res == filesystem::FsResult::Success ||
               res == filesystem::FsResult::AlreadyExists,
           Services, "Failed to create save data directory: {}", res);

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::readSaveDataFileSystemExtraDataBySaveDataSpaceId(
    Aligned<SaveDataSpaceId, 8> space_id, u64 save_id,
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "space ID: {}, save ID: {}", space_id,
                               save_id);

    // TODO: why is the stream NULL?
    if (out_buffer.stream) {
        // HACK
        out_buffer.stream->write<SaveDataFileSystemExtraData>({});
    }
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::openSaveDataFileSystem(
    RequestContext* ctx, System* system, kernel::Process* process,
    Aligned<SaveDataSpaceId, 8> space_id, SaveDataAttribute attr) {
    return openSaveDataFileSystemImpl(ctx, system, process, space_id, attr,
                                      false);
}

result_t IFileSystemProxy::openReadOnlySaveDataFileSystem(
    RequestContext* ctx, System* system, kernel::Process* process,
    Aligned<SaveDataSpaceId, 8> space_id, SaveDataAttribute attr) {
    return openSaveDataFileSystemImpl(ctx, system, process, space_id, attr,
                                      true);
}

// TODO: space ID
result_t IFileSystemProxy::openSaveDataInfoReaderBySaveDataSpaceId(
    RequestContext* ctx, SaveDataSpaceId space_id) {
    (void)space_id;

    addService(*ctx, new ISaveDataInfoReader());
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::openDataStorageByCurrentProcess(
    RequestContext* ctx, System* system, kernel::Process* process) {
    return openDataStorageByProgramId(ctx, system, process->getTitleId());
}

result_t IFileSystemProxy::openDataStorageByProgramId(RequestContext* ctx,
                                                      System* system,
                                                      u64 program_id) {
    LOG_DEBUG(Services, "Program ID: 0x{:016x}", program_id);

    // TODO: program ID
    (void)program_id;

    filesystem::IFile* file = nullptr;
    const auto res =
        system->getOs().getFilesystem().getFile(FS_SD_MOUNT "/rom/romFS", file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Data storage does not exist");
        return MAKE_RESULT(Fs, res);
    }

    addService(*ctx, new IStorage(file, filesystem::FileOpenFlags::Read));

    return RESULT_SUCCESS;
}

result_t
IFileSystemProxy::openDataStorageByDataId(RequestContext* ctx, System* system,
                                          Aligned<ncm::StorageID, 8> storage_id,
                                          u64 data_id) {
    LOG_FUNC_NOT_IMPLEMENTED(Services);

    LOG_DEBUG(Services, "Storage ID: {}, data ID: 0x{:016x}", storage_id,
              data_id);

    filesystem::IFile* file;
    // NOLINTNEXTLINE(readability-trivial-switch)
    switch (storage_id.get()) {
    case ncm::StorageID::BuiltInSystem: {
        // TODO: correct?
        const auto res = system->getOs().getFilesystem().getFile(
            fmt::format(FS_FIRMWARE_PATH "/{:016x}/public data", data_id),
            file);
        ASSERT(res == filesystem::FsResult::Success, Services,
               "Failed to get built-in system data storage {:016x}: {}",
               data_id, res);
        break;
    }
    default:
        LOG_NOT_IMPLEMENTED(Services, "Storage ID {} (data ID: 0x{:016x})",
                            storage_id, data_id);
        return MAKE_RESULT(Svc, 1); // TODO
    }

    filesystem::ContentArchive content_archive(file);
    const auto res = content_archive.getFile("data", file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Data storage does not have romFS");
        return MAKE_RESULT(Fs, res);
    }

    addService(*ctx, new IStorage(file, filesystem::FileOpenFlags::Read));

    return RESULT_SUCCESS;
}

result_t
IFileSystemProxy::openPatchDataStorageByCurrentProcess(RequestContext* ctx,
                                                       System* system) {
    LOG_NOT_IMPLEMENTED(Services, "OpenPatchDataStorageByCurrentProcess");

    // HACK
    filesystem::IFile* file = nullptr;
    const auto res =
        system->getOs().getFilesystem().getFile(FS_SD_MOUNT "/rom/romFS", file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Data storage does not exist");
        return MAKE_RESULT(Fs, res);
    }

    addService(*ctx, new IStorage(file, filesystem::FileOpenFlags::Read));

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::disableAutoSaveDataCreation() {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::getGlobalAccessLogMode(u32* out_log_mode) {
    LOG_FUNC_STUBBED(Services);

    // TODO: what should this be?
    *out_log_mode = 0;
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::openSaveDataFileSystemImpl(
    RequestContext* ctx, System* system, kernel::Process* process,
    SaveDataSpaceId space_id, SaveDataAttribute attr, bool read_only) {
    (void)space_id;
    (void)read_only;

    // TODO: support read only

    std::string mount = getSaveDataMount(process, attr);
    addService(*ctx, new IFileSystem(mount));

    // TODO: correct?
    const auto res =
        system->getOs().getFilesystem().createDirectory(mount, true);
    ASSERT_DEBUG(res == filesystem::FsResult::Success ||
                     res == filesystem::FsResult::AlreadyExists,
                 Services, "Failed to create save data directory: {}", res);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
