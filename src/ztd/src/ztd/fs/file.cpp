#include "ztd/fs/file.hpp"

namespace ztd::fs {

auto File::getPosixOpenFlags(OpenFlags flags) noexcept -> i32 {
    i32 res = 0;

    // Access mode
    if (any(flags & (OpenFlags::Read | OpenFlags::Write))) {
        res |= O_RDWR;
    } else if (any(flags & OpenFlags::Read)) {
        res |= O_RDONLY;
    } else if (any(flags & OpenFlags::Write)) {
        res |= O_WRONLY;
    }

    // Behavior
    if (any(flags & OpenFlags::Append)) {
        res |= O_APPEND;
    }

    return res;
}

auto File::getPosixCreateFlags(CreateFlags flags) noexcept -> i32 {
    i32 res = O_CREAT;

    // Access mode
    if (any(flags & CreateFlags::Read)) {
        res |= O_RDWR;
    } else {
        res |= O_WRONLY;
    }

    // Behavior
    if (any(flags & CreateFlags::Exclusive)) {
        res |= O_EXCL;
    }

    if (any(flags & CreateFlags::Truncate)) {
        res |= O_TRUNC;
    }

    return res;
}

} // namespace ztd::fs
