#pragma once

// TODO: remove
#include <chrono>
#include <thread>

#include "ztd/type_aliases.hpp"

#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)
#include <x86intrin.h>
#endif

using namespace std::chrono_literals;

namespace ztd {

#if defined(__x86_64__) || defined(_M_X64) || defined(__amd64__)

inline auto getSystemTick() noexcept -> u64 {
    _mm_lfence();
    u64 res = __rdtsc();
    _mm_lfence();
    return res;
}

// TODO: rework
inline auto getSystemFrequency() noexcept -> u64 {
    auto nsc_start = std::chrono::steady_clock::now().time_since_epoch();
    u64 tsc_start = getSystemTick();
    // More sleep, more precision.
    std::this_thread::sleep_for(10ms);
    auto nsc_end = std::chrono::steady_clock::now().time_since_epoch();
    u64 tsc_end = getSystemTick();
    u64 ns_diff =
        static_cast<u64>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                             nsc_end - nsc_start)
                             .count());
    u64 res = (tsc_end - tsc_start) * 1000000000ull / (ns_diff);
    res = res + 100'000 / 2;
    res -= res % 100'000;
    return res;
}

#elif defined(_M_ARM64) || defined(__aarch64__)

inline auto getSystemTick() noexcept -> u64 {
    u64 res;
    __asm__ __volatile__("mrs %0, cntvct_el0; " : "=r"(res)::"memory");
    return res;
}

inline auto getSystemFrequency() noexcept -> u64 {
    u64 res;
    __asm__ __volatile__("mrs %0, cntfrq_el0; isb; " : "=r"(res)::"memory");
    return res;
}

#endif

} // namespace ztd
