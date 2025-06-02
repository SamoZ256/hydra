#pragma once

namespace hydra::horizon::services::fssrv {

constexpr usize MAX_PATH_SIZE = 0x301;

enum class EntryType : u32 {
    Directory,
    File,
};

enum class DirectoryFilterFlags {
    None = 0,
    Directories = BIT(0),
    Files = BIT(1),
};
ENABLE_ENUM_BITMASK_OPERATORS(DirectoryFilterFlags)

enum class SaveDataType : u8 {
    System = 0,
    Account = 1,
    Bcat = 2,
    Device = 3,
    Temporary = 4,
    Cache = 5,
    SystemBcat = 6,
};

enum class SaveDataSpaceId : u8 {
    System = 0,
    User = 1,
    SdSystem = 2,
    Temporary = 3,
    SdUser = 4,
    ProperSystem = 100,
    SafeMode = 101,
};

enum class SaveDataRank : u8 {
    Primary = 0,
    Secondary = 1,
};

} // namespace hydra::horizon::services::fssrv

ENABLE_ENUM_FLAGS_FORMATTING(
    hydra::horizon::services::fssrv::DirectoryFilterFlags, Directories,
    "directories", Files, "files")
