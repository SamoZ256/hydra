#pragma once

#include "ztd/platform.hpp"

namespace ztd::inline builtin {

[[noreturn]] inline void unreachable() {
#if defined(ZTD_COMPILER_CLANG) && !defined(ZTD_COMPILER_GCC)
    __builtin_unreachable();
#elifdef ZTD_COMPILER_MSVC
    __assume(false);
#else
#error "'unreachable' not defined for this compiler"
#endif
}

} // namespace ztd::inline builtin
