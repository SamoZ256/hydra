#pragma once

#include <cerrno>
#include <fcntl.h>
#include <unistd.h>

#include "ztd/builtin.hpp"
#include "ztd/type_aliases.hpp"

namespace ztd::fs::inline internal {

enum class Error : u8 {
    NotFound,
    NotADir,
    AlreadyExists,
    AccessDenied,
    PermissionDenied,
    SymLinkLoop,
    ProcessFdQuotaExceeded,
    SystemFdQuotaExceeded,
    // NoDevice,
    SystemResources,
    // DeviceBusy,
};

inline auto getPosixError() noexcept -> Error {
    switch (errno) {
    case EACCES:
        return Error::AccessDenied;
    case EROFS:
        return Error::AccessDenied; // TODO: correct?
    case EPERM:
        return Error::PermissionDenied;
    case ENOENT:
        return Error::NotFound;
    case ENOTDIR:
        return Error::NotADir;
    case EISDIR:
        return Error::AccessDenied; // TODO: correct?
    case ENAMETOOLONG:
        unreachable(); // Prevented by the caller
    case ELOOP:
        return Error::SymLinkLoop;
    case EMFILE:
        return Error::ProcessFdQuotaExceeded;
    case ENFILE:
        return Error::SystemFdQuotaExceeded;
    case ENOSPC:
    case EDQUOT:
        return Error::SystemResources;
    case EEXIST:
        return Error::AlreadyExists;
    default:
        // TODO: panic
        unreachable();
    }
}

} // namespace ztd::fs::inline internal
