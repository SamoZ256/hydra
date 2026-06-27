#pragma once

namespace ztd::mem {

constexpr auto operator""_KiB(unsigned long long x) noexcept
    -> unsigned long long {
    return x * 1024;
}

constexpr auto operator""_MiB(unsigned long long x) noexcept
    -> unsigned long long {
    return x * 1024_KiB;
}

constexpr auto operator""_GiB(unsigned long long x) noexcept
    -> unsigned long long {
    return x * 1024_MiB;
}

constexpr auto operator""_TiB(unsigned long long x) noexcept
    -> unsigned long long {
    return x * 1024_GiB;
}

} // namespace ztd::mem
