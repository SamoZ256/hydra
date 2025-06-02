#pragma once

#include "common/type_aliases.hpp"

inline unsigned long long operator"" _KiB(unsigned long long x) {
    return x * 1024;
}

inline unsigned long long operator"" _MiB(unsigned long long x) {
    return x * 1024_KiB;
}

inline unsigned long long operator"" _GiB(unsigned long long x) {
    return x * 1024_MiB;
}

inline unsigned long long operator"" _TiB(unsigned long long x) {
    return x * 1024_GiB;
}
