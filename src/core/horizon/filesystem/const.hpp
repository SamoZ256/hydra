#pragma once

#define FS_SD_MOUNT "sdmc:"
#define FS_SAVE_MOUNT "save:"
#define FS_CACHE_MOUNT "cache:"

#define FS_SAVE_DATA_PATH(title_id, user_id)                                   \
    fmt::format(FS_SAVE_MOUNT "/{:016x}/{:032x}", title_id, user_id)

namespace hydra::horizon::filesystem {

enum class FsResult {
    Success,

    NotMounted,
    DoesNotExist,
    IntermediateDirectoryDoesNotExist,
    NotAFile,
    NotADirectory,
    AlreadyExists,
    DirectoryNotEmpty,

    NotImplemented,
};

enum class FileOpenFlags {
    None = 0,
    Read = BIT(0),
    Write = BIT(1),
    Append = BIT(2),
};
ENABLE_ENUM_BITMASK_OPERATORS(FileOpenFlags)

} // namespace hydra::horizon::filesystem

ENABLE_ENUM_FORMATTING(hydra::horizon::filesystem::FsResult, Success, "success",
                       NotMounted, "not mounted", DoesNotExist,
                       "does not exist", IntermediateDirectoryDoesNotExist,
                       "intermediate directory does not exist", NotAFile,
                       "not a file", NotADirectory, "not a directory",
                       AlreadyExists, "already exists", DirectoryNotEmpty,
                       "directory not empty", NotImplemented, "not implemented")

ENABLE_ENUM_FLAGS_FORMATTING(hydra::horizon::filesystem::FileOpenFlags, Read,
                             "read", Write, "write", Append, "append")
