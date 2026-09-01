#pragma once

#include "ztd/fs/file.hpp"

namespace ztd::fs {

class Directory {
  public:
    enum class OpenFlags : u8 {
        None = 0,
        DontFollowSymlinks = ZTD_BIT(0),
    };

    Directory(i32 handle_) noexcept : handle{handle_} {}
    ~Directory() noexcept {
        if (handle >= 0)
            close(handle);
    }

    ZTD_MAKE_NON_COPYABLE(Directory);
    ZTD_MAKE_MOVABLE(Directory, handle, std::exchange(other.handle, -1));

    // TODO: allow std::string_view
    [[nodiscard]] auto openDirectory(const std::string& rel_path,
                                     OpenFlags flags) const noexcept
        -> std::expected<Directory, Error>;

    // TODO: allow std::string_view
    [[nodiscard]] auto openFile(const std::string& rel_path,
                                File::OpenFlags flags) const noexcept
        -> std::expected<File, Error>;

    // TODO: allow std::string_view
    [[nodiscard]] auto createFile(const std::string& rel_path,
                                  File::CreateFlags flags) const noexcept
        -> std::expected<File, Error>;

    [[nodiscard]] auto getHandle() const noexcept -> i32 { return handle; }

  private:
    i32 handle;

    // Helpers
    static auto getPosixOpenFlags(OpenFlags flags) noexcept -> i32;
};

ZTD_ENABLE_ENUM_BITWISE_OPERATORS(Directory::OpenFlags);

[[nodiscard]] constexpr auto cwd() noexcept -> Directory { return {AT_FDCWD}; }

// TODO: allow std::string_view
[[nodiscard]] auto openDirectoryAbsolute(const std::string& rel_path,
                                         Directory::OpenFlags flags) noexcept
    -> std::expected<Directory, Error>;

// TODO: allow std::string_view
[[nodiscard]] auto openFileAbsolute(const std::string& rel_path,
                                    File::OpenFlags flags) noexcept
    -> std::expected<File, Error>;

// TODO: allow std::string_view
[[nodiscard]] auto createFileAbsolute(const std::string& rel_path,
                                      File::CreateFlags flags) noexcept
    -> std::expected<File, Error>;

} // namespace ztd::fs
