#pragma once

#include <expected>
#include <string>

#include "ztd/fs/error.hpp"
#include "ztd/macros/constructor_helper.hpp"
#include "ztd/macros/enum_helper.hpp"

namespace ztd::fs {

class File {
    friend class Directory;

  public:
    enum class OpenFlags : u8 {
        None = 0,
        Read = ZTD_BIT(0),
        Write = ZTD_BIT(1),
        Append = ZTD_BIT(2),
    };

    enum class CreateFlags : u8 {
        None = 0,
        Read = ZTD_BIT(0),
        Append = ZTD_BIT(1),
        Exclusive = ZTD_BIT(2),
        Truncate = ZTD_BIT(3),
    };

    File() noexcept = default;
    File(i32 handle_) noexcept : handle{handle_} {}
    ~File() noexcept {
        if (handle >= 0)
            close(handle);
    }

    ZTD_MAKE_NON_COPYABLE(File);
    ZTD_MAKE_MOVABLE(File, handle, std::exchange(other.handle, -1));

    [[nodiscard]] auto getHandle() const noexcept -> i32 { return handle; }

  private:
    i32 handle{-1};

    // Helpers
    static auto getPosixOpenFlags(OpenFlags flags) noexcept -> i32;
    static auto getPosixCreateFlags(CreateFlags flags) noexcept -> i32;
};

ZTD_ENABLE_ENUM_BITWISE_OPERATORS(File::OpenFlags);
ZTD_ENABLE_ENUM_BITWISE_OPERATORS(File::CreateFlags);

} // namespace ztd::fs
