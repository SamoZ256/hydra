#pragma once

#include <algorithm>
#include <cassert>
#include <charconv>
#include <cxxabi.h>
#include <fstream>
#include <string>
#include <sys/mman.h>

#include "common/log.hpp"

namespace hydra {

[[noreturn]] inline void unreachable() {
#ifdef HYDRA_DEBUG
    LOG_FATAL(Common, "Unreachable code reached");
#else
    ztd::builtin::unreachable();
#endif
}

template <typename T>
constexpr T allOnes() {
    return std::numeric_limits<T>::max();
}

template <typename T>
constexpr T invalid() {
    return allOnes<T>();
}

template <typename T>
constexpr T mask(u32 b, u32 count) {
    return static_cast<T>(((1ull << count) - 1ull) << b);
}

template <typename T>
T extractBits(T src, u32 b, u32 count) {
    return (src & mask<T>(b, count)) >> b;
}

template <typename T, u32 bit_count>
T signExtend(T v) {
    static_assert(bit_count < sizeof(T) * 8);
    T const m = T(1) << (bit_count - 1);
    return (v ^ m) - m;
}

template <typename T>
T align(T v, T alignment) {
    return (v + alignment - 1) & ~(alignment - 1);
}

template <typename T>
bool isAligned(T v, T alignment) {
    return (v % alignment) == 0x0;
}

template <typename T>
T alignDown(T v, T alignment) {
    return v & ~(alignment - 1);
}

template <typename PtrT, typename AlignmentT>
PtrT* alignPtr(PtrT* ptr, AlignmentT alignment) {
    return reinterpret_cast<PtrT*>(
        align(reinterpret_cast<u64>(ptr), static_cast<u64>(alignment)));
}

inline uptr makeAddr(u32 lo, u32 hi) {
    return (static_cast<uptr>(hi) << 32) | lo;
}

template <typename T>
T ceilDivide(T dividend, T divisor) {
    return (dividend + divisor - 1) / divisor;
}

constexpr u32 makeMagic4(const char c0, const char c1, const char c2,
                         const char c3) {
    return static_cast<u32>(c0) | static_cast<u32>(c1) << 8 |
           static_cast<u32>(c2) << 16 | static_cast<u32>(c3) << 24;
}

inline std::string toLower(const std::string_view str) {
    // TODO: make this more efficient?
    std::string result;
    std::ranges::transform(str, std::back_inserter(result),
                           [](unsigned char c) { return std::tolower(c); });

    return result;
}

inline std::string toUpper(const std::string_view str) {
    // TODO: make this more efficient?
    std::string result;
    std::ranges::transform(str, std::back_inserter(result),
                           [](unsigned char c) { return std::toupper(c); });

    return result;
}

inline std::string demangle(const char* mangled_name) {
    i32 status;
    std::unique_ptr<char, void (*)(void*)> result{
        abi::__cxa_demangle(mangled_name, nullptr, nullptr, &status),
        std::free};

    return (status == 0) ? result.get() : mangled_name;
}

inline std::string demangle(const std::string& mangled_name) {
    return demangle(mangled_name.c_str());
}

template <typename T>
bool strToNum(const std::string_view str, T& value) {
    if (str.empty())
        return false;

    const char* first = str.data();
    const char* last = str.data() + str.length();

    std::from_chars_result res = std::from_chars(first, last, value);

    if (res.ec != std::errc())
        return false;
    if (res.ptr != last)
        return false;

    return true;
}

} // namespace hydra
