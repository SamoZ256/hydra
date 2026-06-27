#pragma once

#include <unistd.h>

#include "ztd/macros/platform.hpp"
#include "ztd/mem/literals.hpp"
#include "ztd/type_aliases.hpp"

namespace ztd::mem {

#ifdef ZTD_PLATFORM_APPLE
#ifdef ZTD_ARCH_AARCH64
// All Apple Silicon devices have fixed 16KiB page size
constexpr usize PAGE_SIZE_MIN = 16_KiB;
constexpr usize PAGE_SIZE_MAX = 16_KiB;
#elifdef ZTD_ARCH_X86_64
// All Intel Macs have fixed 4KiB page size
constexpr usize PAGE_SIZE_MIN = 4_KiB;
constexpr usize PAGE_SIZE_MAX = 4_KiB;
#else
// Fallback
constexpr usize PAGE_SIZE_MIN = 4_KiB;
constexpr usize PAGE_SIZE_MAX = 2_GiB;
#endif
// TODO: other platforms
#else
// Fallback
constexpr usize PAGE_SIZE_MIN = 4_KiB;
constexpr usize PAGE_SIZE_MAX = 2_GiB;
#endif

inline auto getPageSize() noexcept -> usize {
    if constexpr (PAGE_SIZE_MIN == PAGE_SIZE_MAX) {
        return PAGE_SIZE_MIN;
    }

    return static_cast<usize>(sysconf(_SC_PAGESIZE));
}

} // namespace ztd::mem
