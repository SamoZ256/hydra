#include "core/horizon/services/fssrv/filesystem_proxy.hpp"

#include "core/horizon/filesystem/filesystem.hpp"
#include "core/horizon/services/fssrv/file.hpp"
#include "core/horizon/services/fssrv/filesystem.hpp"
#include "core/horizon/services/fssrv/save_data_info_reader.hpp"
#include "core/horizon/services/fssrv/storage.hpp"

namespace hydra::horizon::services::fssrv {

namespace {

std::string get_save_data_mount(const SaveDataAttribute& attr) {
    switch (attr.type) {
    case SaveDataType::Account: {
        u64 title_id = attr.title_id;
        // TODO
        // if (title_id == 0x0)
        //    title_id = KERNEL_INSTANCE.GetTitleID();
        return FS_SAVE_DATA_PATH(title_id, attr.user_id);
    }
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
    200, OpenDataStorageByProgramId, 202, OpenDataStorageByDataId, 203,
    OpenPatchDataStorageByCurrentProcess, 1003, DisableAutoSaveDataCreation,
    1005, GetGlobalAccessLogMode)

result_t IFileSystemProxy::OpenFileSystem(
    RequestContext* ctx, FileSystemProxyType type,
    InBuffer<BufferAttr::HipcPointer> path_buffer) {
    // TODO: correct?
    const auto mount = path_buffer.reader->ReadString();
    LOG_DEBUG(Services, "Mount: {}", mount);

    AddService(*ctx, new IFileSystem(mount));

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenFileSystemWithIdObsolete(
    RequestContext* ctx, FileSystemProxyType type, u64 program_id,
    InBuffer<BufferAttr::HipcPointer> path_buffer) {
    // TODO: correct?
    const auto mount = path_buffer.reader->ReadString();
    LOG_DEBUG(Services, "Mount: {}", mount);

    AddService(*ctx, new IFileSystem(mount));

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenBisFileSystem(
    BisPartitionId partition_id,
    InBuffer<BufferAttr::HipcPointer> unknown_buffer) {
    const auto unknown =
        unknown_buffer.reader->ReadString(); // TODO: what is this for?
    LOG_DEBUG(Services, "Partition ID: {}, unknown: {}", partition_id, unknown);

    LOG_FUNC_STUBBED(Services);

    return MAKE_RESULT(Fs, 1771);
}

result_t IFileSystemProxy::OpenSdCardFileSystem(RequestContext* ctx) {
    // TODO: correct?
    AddService(*ctx, new IFileSystem(FS_SD_MOUNT));
    return RESULT_SUCCESS;
}

result_t
IFileSystemProxy::CreateSaveDataFileSystem(SaveDataAttribute attr,
                                           SaveDataCreationInfo creation_info,
                                           SaveDataMetaInfo meta_info) {
    std::string mount = get_save_data_mount(attr);
    auto res = FILESYSTEM_INSTANCE.CreateDirectory(mount, true);
    // TODO: check res

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::ReadSaveDataFileSystemExtraDataBySaveDataSpaceId(
    aligned<SaveDataSpaceId, 8> space_id, u64 save_id,
    OutBuffer<BufferAttr::MapAlias> out_buffer) {
    LOG_FUNC_STUBBED(Services);

    // TODO: why is the writer NULL?
    if (out_buffer.writer) {
        // HACK
        out_buffer.writer->Write<SaveDataFileSystemExtraData>({});
    }
    return RESULT_SUCCESS;
}

result_t
IFileSystemProxy::OpenSaveDataFileSystem(RequestContext* ctx,
                                         aligned<SaveDataSpaceId, 8> space_id,
                                         SaveDataAttribute attr) {
    return OpenSaveDataFileSystemImpl(ctx, space_id, attr, false);
}

result_t IFileSystemProxy::OpenReadOnlySaveDataFileSystem(
    RequestContext* ctx, aligned<SaveDataSpaceId, 8> space_id,
    SaveDataAttribute attr) {
    return OpenSaveDataFileSystemImpl(ctx, space_id, attr, true);
}

result_t IFileSystemProxy::OpenSaveDataInfoReaderBySaveDataSpaceId(
    RequestContext* ctx, SaveDataSpaceId space_id) {
    // TODO: space ID
    AddService(*ctx, new ISaveDataInfoReader());
    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenDataStorageByProgramId(RequestContext* ctx,
                                                      u64 program_id) {
    LOG_DEBUG(Services, "Program ID: {}", program_id);

    // TODO: program ID

    filesystem::FileBase* file = nullptr;
    const auto res =
        FILESYSTEM_INSTANCE.GetFile(FS_SD_MOUNT "/rom/romFS", file);
    if (res != filesystem::FsResult::Success) {
        LOG_WARN(Services, "Data storage does not exist");
        return MAKE_RESULT(Fs, res);
    }

    AddService(*ctx, new IStorage(file, filesystem::FileOpenFlags::Read));

    return RESULT_SUCCESS;
}

result_t IFileSystemProxy::OpenDataStorageByDataId(
    RequestContext* ctx, aligned<ncm::StorageID, 8> storage_id, u64 data_id) {
    LOG_FUNC_NOT_IMPLEMENTED(Services);

    LOG_DEBUG(Services, "Storage ID: {}, data ID: 0x{:08x}", storage_id.Get(),
              data_id);

    // TODO: implement
    return RESULT_SUCCESS;
}

result_t
IFileSystemProxy::OpenPatchDataStorageByCurrentProcess(RequestContext* ctx) {
    LOG_NOT_IMPLEMENTED(Services, "OpenPatchDataStorageByCurrentProcess");

    // HACK
    filesystem::FileBase* file = nullptr;
    const auto res =
        FILESYSTEM_INSTANCE.GetFile(FS_SD_MOUNT "/rom/romFS", file);
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

result_t IFileSystemProxy::OpenSaveDataFileSystemImpl(RequestContext* ctx,
                                                      SaveDataSpaceId space_id,
                                                      SaveDataAttribute attr,
                                                      bool read_only) {
    // TODO: support read only

    std::string mount = get_save_data_mount(attr);
    AddService(*ctx, new IFileSystem(mount));

    // TODO: correct?
    auto res = FILESYSTEM_INSTANCE.CreateDirectory(mount, true);
    // TODO: check res

    return RESULT_SUCCESS;
}

} // namespace hydra::horizon::services::fssrv
