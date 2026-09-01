#include "ztd/fs/directory.hpp"

namespace ztd::fs {

auto Directory::openDirectory(const std::string& rel_path,
                              OpenFlags flags) const noexcept
    -> std::expected<Directory, Error> {
    const auto fd = openat(handle, rel_path.c_str(), getPosixOpenFlags(flags));
    if (fd == -1)
        return std::unexpected(getPosixError());

    return {fd};
}

auto Directory::openFile(const std::string& rel_path,
                         File::OpenFlags flags) const noexcept
    -> std::expected<File, Error> {
    const auto fd =
        openat(handle, rel_path.c_str(), File::getPosixOpenFlags(flags));
    if (fd == -1)
        return std::unexpected(getPosixError());

    return {fd};
}

auto Directory::createFile(const std::string& rel_path,
                           File::CreateFlags flags) const noexcept
    -> std::expected<File, Error> {
    const auto fd =
        openat(handle, rel_path.c_str(), File::getPosixCreateFlags(flags),
               0644); // TODO: what should the permissions be?
    if (fd == -1)
        return std::unexpected(getPosixError());

    return {fd};
}

auto Directory::getPosixOpenFlags(OpenFlags flags) noexcept -> i32 {
    // TODO: O_PATH?
    i32 res = O_DIRECTORY | O_RDONLY | O_CLOEXEC;
    if (any(flags & OpenFlags::DontFollowSymlinks)) {
        res |= O_NOFOLLOW;
    }

    return res;
}

auto openDirectoryAbsolute(const std::string& path,
                           Directory::OpenFlags flags) noexcept
    -> std::expected<Directory, Error> {
    // TODO: verify the path is absolute?
    return cwd().openDirectory(path, flags);
}

auto openFileAbsolute(const std::string& path, File::OpenFlags flags) noexcept
    -> std::expected<File, Error> {
    // TODO: verify the path is absolute?
    return cwd().openFile(path, flags);
}

auto createFileAbsolute(const std::string& path,
                        File::CreateFlags flags) noexcept
    -> std::expected<File, Error> {
    // TODO: verify the path is absolute?
    return cwd().createFile(path, flags);
}

} // namespace ztd::fs
