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

std::string get_save_data_mount(kernel::Process* process,
                                const SaveDataAttribute& attr) {
    u64 title_id = attr.title_id;
    // TODO: is this correct?
    if (title_id == 0x0)
        title_id = process->GetTitleID();

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
    IFileSystemProxy, 0, OpenFileSystem, 1, SetCurrentProcess, 8,
    OpenFileSystemWithIdObsolete, 11, OpenBisFileSystem, 18,
    OpenSdCardFileSystem, 22, CreateSaveDataFileSystem, 51,
    OpenSaveDataFileSystem, 52,
    ReadSaveDataFileSystemExtraDataBySaveDataSpaceId, 53,
    OpenReadOnlySaveDataFileSystem, 61, OpenSaveDataInfoReaderBySaveDataSpaceId,
    200, OpenDataStorageByCurrentProcess, 201, OpenDataStorageByProgramId, 202,
    OpenDataStorageByDataId, 203, OpenPatchDataStorageByCurrentProcess, 1003,
    DisableAutoSaveDataCreation, 1005, GetGlobalAccessLogMode)

// TODO: type
result_t IFileSystemProxy::OpenFileSystem(
    RequestContext* ctx, FileSystemProxyType type,
    InBuffer<BufferAttr::HipcPointer> path_buffer) {
    (void)type;

    // TODO: correct?
    const auto mount = path_buffer.stream->ReadNullTerminatedString();
    LOG_DEBUG(Services, "Mount: {}", mount);

    AddService(*ctx, new IFileSystem(mount));

    return RESULT_SUCCESS;
}

// TODO: type
result_t IFileSystemProxy::OpenFileSystemWithIdObsolete(
    RequestContext* ctx, FileSystemProxyType type, u64 program_id,
    InBuffer<BufferAttr::HipcPointer> path_buffer) {
    (void)type;
    (void)program_id;

    // TODO: correct?
    const auto mount = path_buffer.stream->ReadNullTerminatedString();
    LOG_DEBUG(Services, "Mount: {}", mount);

    AddService(*ctx, new IFileSystem(mount));

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenBisFileSystem(
    BisPartitionId partition_id,
    InBuffer<BufferAttr::HipcPointer> unknown_buffer) {
    const auto unknown =
        unknown_buffer.stream
            ->ReadNullTerminatedString(); // TODO: what is this for?

    LOG_FUNC_WITH_ARGS_STUBBED(Services, "partition ID: {}, unknown: {}",
                               partition_id, unknown);

    return MAKE_RESULT(Fs, 1771);
}

result_t IFileSystemProxy::OpenSdCardFileSystem(RequestContext* ctx) {
    // TODO: correct?
    AddService(*ctx, new IFileSystem(FS_SD_MOUNT));
    return RESULT_SUCCESS;
}

// TODO: creation and meta info
result_t IFileSystemProxy::CreateSaveDataFileSystem(
    System* system, kernel::Process* process, SaveDataAttribute attr,
    SaveDataCreationInfo creation_info, SaveDataMetaInfo meta_info) {
    (void)creation_info;
    (void)meta_info;

    std::string mount = get_save_data_mount(process, attr);
    const auto res =
        system->GetOS().GetFilesystem().CreateDirectory(mount, true);
    ASSERT(res == filesystem::FsResult::Success ||
               res == filesystem::FsResult::AlreadyExists,
           Services, "Failed to create save data directory: {}", res);

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::ReadSaveDataFileSystemExtraDataBySaveDataSpaceId(
    aligned<SaveDataSpaceId, 8> space_id, u64 save_id,
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    LOG_FUNC_WITH_ARGS_STUBBED(Services, "space ID: {}, save ID: {}", space_id,
                               save_id);

    // TODO: why is the stream NULL?
    if (out_buffer.stream) {
        // HACK
        out_buffer.stream->Write<SaveDataFileSystemExtraData>({});
    }
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenSaveDataFileSystem(
    RequestContext* ctx, System* system, kernel::Process* process,
    aligned<SaveDataSpaceId, 8> space_id, SaveDataAttribute attr) {
    return OpenSaveDataFileSystemImpl(ctx, system, process, space_id, attr,
                                      false);
}

result_t IFileSystemProxy::OpenReadOnlySaveDataFileSystem(
    RequestContext* ctx, System* system, kernel::Process* process,
    aligned<SaveDataSpaceId, 8> space_id, SaveDataAttribute attr) {
    return OpenSaveDataFileSystemImpl(ctx, system, process, space_id, attr,
                                      true);
}

// TODO: space ID
result_t IFileSystemProxy::OpenSaveDataInfoReaderBySaveDataSpaceId(
    RequestContext* ctx, SaveDataSpaceId space_id) {
    (void)space_id;

    AddService(*ctx, new ISaveDataInfoReader());
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenDataStorageByCurrentProcess(
    RequestContext* ctx, System* system, kernel::Process* process) {
    return OpenDataStorageByProgramId(ctx, system, process->GetTitleID());
}

result_t IFileSystemProxy::OpenDataStorageByProgramId(RequestContext* ctx,
                                                      System* system,
                                                      u64 program_id) {
    LOG_DEBUG(Services, "Program ID: 0x{:016x}", program_id);

    // TODO: program ID
    (void)program_id;

    filesystem::IFile* file = nullptr;
    const auto res =
        system->GetOS().GetFilesystem().GetFile(FS_SD_MOUNT "/rom/romFS", file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Data storage does not exist");
        return MAKE_RESULT(Fs, res);
    }

    AddService(*ctx, new IStorage(file, filesystem::FileOpenFlags::Read));

    return RESULT_SUCCESS;
}

result_t
IFileSystemProxy::OpenDataStorageByDataId(RequestContext* ctx, System* system,
                                          aligned<ncm::StorageID, 8> storage_id,
                                          u64 data_id) {
    LOG_FUNC_NOT_IMPLEMENTED(Services);

    LOG_DEBUG(Services, "Storage ID: {}, data ID: 0x{:016x}", storage_id,
              data_id);

    filesystem::IFile* file;
    switch (storage_id.Get()) {
    case ncm::StorageID::BuiltInSystem: {
        // TODO: correct?
        const auto res = system->GetOS().GetFilesystem().GetFile(
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
    const auto res = content_archive.GetFile("data", file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Data storage does not have romFS");
        return MAKE_RESULT(Fs, res);
    }

    AddService(*ctx, new IStorage(file, filesystem::FileOpenFlags::Read));

    return RESULT_SUCCESS;
}

result_t
IFileSystemProxy::OpenPatchDataStorageByCurrentProcess(RequestContext* ctx,
                                                       System* system) {
    LOG_NOT_IMPLEMENTED(Services, "OpenPatchDataStorageByCurrentProcess");

    // HACK
    filesystem::IFile* file = nullptr;
    const auto res =
        system->GetOS().GetFilesystem().GetFile(FS_SD_MOUNT "/rom/romFS", file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Data storage does not exist");
        return MAKE_RESULT(Fs, res);
    }

    AddService(*ctx, new IStorage(file, filesystem::FileOpenFlags::Read));

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::DisableAutoSaveDataCreation() {
    LOG_FUNC_STUBBED(Services);
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::GetGlobalAccessLogMode(u32* out_log_mode) {
    LOG_FUNC_STUBBED(Services);

    // TODO: what should this be?
    *out_log_mode = 0;
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenSaveDataFileSystemImpl(
    RequestContext* ctx, System* system, kernel::Process* process,
    SaveDataSpaceId space_id, SaveDataAttribute attr, bool read_only) {
    (void)space_id;
    (void)read_only;

    // TODO: support read only

    std::string mount = get_save_data_mount(process, attr);
    AddService(*ctx, new IFileSystem(mount));

    // TODO: correct?
    const auto res =
        system->GetOS().GetFilesystem().CreateDirectory(mount, true);
    ASSERT_DEBUG(res == filesystem::FsResult::Success ||
                     res == filesystem::FsResult::AlreadyExists,
                 Services, "Failed to create save data directory: {}", res);

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
