#pragma once

#include "core/horizon/services/const.hpp"
#include "core/horizon/services/fssrv/const.hpp"
#include "core/horizon/services/ncm/const.hpp"

namespace hydra::horizon::services::fssrv {

enum class FileSystemProxyType {
    Code,
    Rom,
    Logo,
    Control,
    Manual,
    Meta,
    Data,
    Package,
    RegisteredUpdate,
};

enum BisPartitionId : u32 {
    BootPartition1Root = 0,

    BootPartition2Root = 10,

    UserDataRoot = 20,
    BootConfigAndPackage2Part1 = 21,
    BootConfigAndPackage2Part2 = 22,
    BootConfigAndPackage2Part3 = 23,
    BootConfigAndPackage2Part4 = 24,
    BootConfigAndPackage2Part5 = 25,
    BootConfigAndPackage2Part6 = 26,
    CalibrationBinary = 27,
    CalibrationFile = 28,
    SafeMode = 29,
    User = 30,
    System_ = 31,
    SystemProperEncryption = 32,
    SystemProperPartition = 33,
    SignedSystemPartitionOnSafeMode = 34,
    DeviceTreeBlob = 35,
    System0 = 36,
};

enum class SaveDataFlags : u32 {
    KeepAfterResettingSystemSaveData = BIT(0),
    KeepAfterRefurbishment = BIT(1),
    KeepAfterResettingSystemSaveDataWithoutUserSaveData = BIT(2),
    NeedsSecureDelete = BIT(3),
};

enum class SaveDataMetaType : u8 {
    None = 0,
    Thumbnail = 1,
    ExtensionContext = 2,
};

#pragma pack(push, 1)
struct SaveDataAttribute {
    u64 title_id;
    uuid_t user_id;
    u64 system_save_data_id;
    SaveDataType type;
    SaveDataRank rank;
    u16 index;
    u32 pad_x24;
    u64 unk_x28;
    u64 unk_x30;
    u64 unk_x38;
};

struct SaveDataCreationInfo {
    i64 save_data_size;
    i64 journal_size;
    u64 available_size;
    u64 owner_id;
    SaveDataFlags flags;
    SaveDataSpaceId space_id;
    u8 unk;
    u8 padding[0x1a];
};

struct SaveDataMetaInfo {
    u32 size;
    SaveDataMetaType type;
    u8 reserved[0x0B];
};

struct SaveDataFileSystemExtraData {
    SaveDataAttribute attr;
    u64 owner_id;
    u64 timestamp;
    u32 flags; // ?
    u32 _unused_x54;
    u64 usable_size;
    u64 journal_size;
    u64 commit_id;
    u8 _padding_x70[0x190];
};
#pragma pack(pop)

class IFileSystemProxy : public IService {
  protected:
    result_t RequestImpl([[maybe_unused]] RequestContext& context,
                         u32 id) override;

  private:
    // Commands
    result_t OpenFileSystem(RequestContext* ctx, FileSystemProxyType type,
                            InBuffer<BufferAttr::HipcPointer> path_buffer);
    STUB_REQUEST_COMMAND(SetCurrentProcess);
    result_t
    OpenFileSystemWithIdObsolete(RequestContext* ctx, FileSystemProxyType type,
                                 u64 program_id,
                                 InBuffer<BufferAttr::HipcPointer> path_buffer);
    result_t
    OpenBisFileSystem(BisPartitionId partition_id,
                      InBuffer<BufferAttr::HipcPointer> unknown_buffer);
    result_t OpenSdCardFileSystem(RequestContext* ctx);
    result_t CreateSaveDataFileSystem(System* system, kernel::Process* process,
                                      SaveDataAttribute attr,
                                      SaveDataCreationInfo creation_info,
                                      SaveDataMetaInfo meta_info);
    result_t ReadSaveDataFileSystemExtraDataBySaveDataSpaceId(
        aligned<SaveDataSpaceId, 8> space_id, u64 save_id,
        OutBuffer<BufferAttr::MapAlias> out_buffer);
    result_t OpenSaveDataFileSystem(RequestContext* ctx, System* system,
                                    kernel::Process* process,
                                    aligned<SaveDataSpaceId, 8> space_id,
                                    SaveDataAttribute attr);
    result_t OpenReadOnlySaveDataFileSystem(
        RequestContext* ctx, System* system, kernel::Process* process,
        aligned<SaveDataSpaceId, 8> space_id, SaveDataAttribute attr);
    result_t OpenSaveDataInfoReaderBySaveDataSpaceId(RequestContext* ctx,
                                                     SaveDataSpaceId space_id);
    result_t OpenDataStorageByCurrentProcess(RequestContext* ctx,
                                             System* system,
                                             kernel::Process* process);
    result_t OpenDataStorageByProgramId(RequestContext* ctx, System* system,
                                        u64 program_id);
    result_t OpenDataStorageByDataId(RequestContext* ctx, System* system,
                                     aligned<ncm::StorageID, 8> storage_id,
                                     u64 data_id);
    result_t OpenPatchDataStorageByCurrentProcess(RequestContext* ctx,
                                                  System* system);
    result_t DisableAutoSaveDataCreation();
    result_t GetGlobalAccessLogMode(u32* out_log_mode);

    // Impl
    result_t OpenSaveDataFileSystemImpl(RequestContext* ctx, System* system,
                                        kernel::Process* process,
                                        SaveDataSpaceId space_id,
                                        SaveDataAttribute attr, bool read_only);
};

} // namespace hydra::horizon::services::fssrv

ENABLE_ENUM_FORMATTING(hydra::horizon::services::fssrv::FileSystemProxyType,
                       Code, "code", Rom, "rom", Logo, "logo", Control,
                       "control", Manual, "manual", Meta, "meta", Data, "data",
                       Package, "package", RegisteredUpdate,
                       "registered update")

ENABLE_ENUM_FORMATTING(
    hydra::horizon::services::fssrv::BisPartitionId, BootPartition1Root,
    "boot partition 1 root", BootPartition2Root, "boot partition 2 root",
    UserDataRoot, "user data root", BootConfigAndPackage2Part1,
    "boot config and package 2 part 1", BootConfigAndPackage2Part2,
    "boot config and package 2 part 2", BootConfigAndPackage2Part3,
    "boot config and package 2 part 3", BootConfigAndPackage2Part4,
    "boot config and package 2 part 4", BootConfigAndPackage2Part5,
    "boot config and package 2 part 5", BootConfigAndPackage2Part6,
    "boot config and package 2 part 6", CalibrationBinary, "calibration binary",
    CalibrationFile, "calibration file", SafeMode, "safe mode", User, "user",
    System_, "system", SystemProperEncryption, "system proper encryption",
    SystemProperPartition, "system proper partition",
    SignedSystemPartitionOnSafeMode, "signed system partition on safe mode",
    DeviceTreeBlob, "device tree blob", System0, "system 0")
