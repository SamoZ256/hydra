#pragma once

#define FS_SD_MOUNT "sdmc:"
#define FS_SAVE_MOUNT "save:"
#define FS_CACHE_MOUNT "cache:"
#define FS_CONTENT_MOUNT "content:"

#define FS_USER_SAVE_DATA_PATH(title_id, user_id)                              \
    fmt::format(FS_SAVE_MOUNT "/{:016x}/{:032x}", title_id, user_id)
#define FS_SHARED_SAVE_DATA_PATH(title_id)                                     \
    fmt::format(FS_SAVE_MOUNT "/{:016x}/shared", title_id)
#define FS_FIRMWARE_PATH FS_CONTENT_MOUNT "/firmware"
#define FS_SYSMODULES_PATH FS_SD_MOUNT "/atmosphere/contents"

namespace hydra::horizon::filesystem {

enum class FsResult {
    Success,

    NotMounted,
    DoesNotExist,
    NotAFile,
    NotADirectory,
    AlreadyExists,
    DirectoryNotEmpty,

    NotImplemented,
};

enum class FileOpenFlags {
    None = 0,
    Read = ZTD_BIT(0),
    Write = ZTD_BIT(1),
    Append = ZTD_BIT(2),
};
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(FileOpenFlags)

} // namespace hydra::horizon::filesystem

ENABLE_ENUM_FORMATTING(hydra::horizon::filesystem::FsResult, Success, "success",
                       NotMounted, "not mounted", DoesNotExist,
                       "does not exist", NotAFile, "not a file", NotADirectory,
                       "not a directory", AlreadyExists, "already exists",
                       DirectoryNotEmpty, "directory not empty", NotImplemented,
                       "not implemented")

ENABLE_ENUM_FLAGS_FORMATTING(hydra::horizon::filesystem::FileOpenFlags, Read,
                             "read", Write, "write", Append, "append")
