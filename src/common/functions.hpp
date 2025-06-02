#pragma once

#include <assert.h>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/mman.h>

#include "common/type_aliases.hpp"

namespace hydra {

[[noreturn]] inline void unreachable() {
#if defined(_MSC_VER) && !defined(__clang__) // MSVC
    __assume(false);
#else // GCC, Clang
    __builtin_unreachable();
#endif
}

template <typename T> constexpr T all_ones() {
    return std::numeric_limits<T>::max();
}

template <typename T> constexpr T invalid() { return all_ones<T>(); }

template <typename T, u64 b, u64 count> constexpr T mask() {
    return ((1u << count) - 1u) << b;
}

template <typename T, u64 b, u64 count, typename SrcT>
T extract_bits(SrcT src) {
    return static_cast<T>((src >> b) & mask<T, 0, count>());
}

template <typename T, u32 bit_count> T sign_extend(T v) {
    static_assert(std::is_signed<T>::value);
    static_assert(bit_count < sizeof(T) * 8);
    T const m = T(1) << (bit_count - 1);
    return (v ^ m) - m;
}

template <typename T> T align(T v, T alignment) {
    return (v + alignment - 1) & ~(alignment - 1);
}

template <typename T> bool is_aligned(T v, T alignment) {
    return (v & (alignment - 1)) == 0x0;
}

template <typename T> T align_down(T v, T alignment) {
    return v & ~(alignment - 1);
}

template <typename PtrT, typename AlignmentT>
PtrT* align_ptr(PtrT* ptr, AlignmentT alignment) {
    return reinterpret_cast<PtrT*>(
        align(reinterpret_cast<u64>(ptr), static_cast<u64>(alignment)));
}

inline uptr make_addr(u32 lo, u32 hi) {
    return (static_cast<uptr>(hi) << 32) | lo;
}

template <typename T> T ceil_divide(T dividend, T divisor) {
    return (dividend + divisor - 1) / divisor;
}

template <typename T> T random() { return rand() & all_ones<T>(); }

inline u64 random64() { return (u64)rand() | ((u64)rand() << 32); }

inline u128 random128() {
    return (u128)rand() | ((u128)rand() << 32) | ((u128)rand() << 64) |
           ((u128)rand() << 96);
}

inline constexpr u32 make_magic4(const char c0, const char c1, const char c2,
                                 const char c3) {
    return (u32)c0 | (u32)c1 << 8 | (u32)c2 << 16 | (u32)c3 << 24;
}

inline std::string to_lower(const std::string_view str) {
    // TODO: make this more efficient?
    std::string result;
    std::transform(str.begin(), str.end(), std::back_inserter(result),
                   [](unsigned char c) { return std::tolower(c); });

    return result;
}

inline std::string to_upper(const std::string_view str) {
    // TODO: make this more efficient?
    std::string result;
    std::transform(str.begin(), str.end(), std::back_inserter(result),
                   [](unsigned char c) { return std::toupper(c); });

    return result;
}

inline constexpr uint64_t str_to_u64(const char* str, size_t idx = 0,
                                     uint64_t result = 0) {
    return (str[idx] == '\0' || idx >= 8)
               ? result
               : str_to_u64(str, idx + 1,
                            result |
                                (static_cast<uint64_t>(str[idx]) << (idx * 8)));
}

inline std::string u64_to_str(u64 value) {
    char* str = reinterpret_cast<char*>(&value);
    return std::string(str, std::min(strlen(str), (usize)8));
}

template <typename T> void push_unique(std::vector<T>& vec, T value) {
    auto it = std::find_if(vec.begin(), vec.end(),
                           [&](const T v) { return v == value; });
    if (it == vec.end())
        vec.push_back(value);
}

// HACK
template <typename T> T rotl(T v, u64 shift) {
    return (v << shift) | (v >> (32 - shift));
}

} // namespace hydra
