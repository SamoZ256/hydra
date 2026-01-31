#pragma once

#include "type_aliases.hpp"

#if __cpp_lib_atomic_ref >= 201806L
#define HYDRA_HAS_ATOMIC_REF 1
#else
#define HYDRA_HAS_ATOMIC_REF 0
#endif

namespace hydra {

#if HYDRA_HAS_ATOMIC_REF

template <typename T>
void atomic_store(T* ptr, T value) {
    std::atomic_ref ref(*ptr);
    ref.store(value);
}

template <typename T>
T atomic_load(T* ptr) {
    std::atomic_ref ref(*ptr);
    return ref.load();
}

template <typename T>
T atomic_exchange(T* ptr, T value) {
    std::atomic_ref ref(*ptr);
    return ref.exchange(value);
}

template <typename T>
bool atomic_compare_exchange_weak(T* ptr, T& expected, T desired) {
    std::atomic_ref ref(*ptr);
    return ref.compare_exchange_weak(expected, desired);
}

template <typename T>
bool atomic_compare_exchange_strong(T* ptr, T& expected, T desired) {
    std::atomic_ref ref(*ptr);
    return ref.compare_exchange_strong(expected, desired);
}

template <typename T>
T atomic_fetch_add(T* ptr, T value) {
    std::atomic_ref ref(*ptr);
    return ref.fetch_add(value);
}

template <typename T>
T atomic_fetch_sub(T* ptr, T value) {
    std::atomic_ref ref(*ptr);
    return ref.fetch_sub(value);
}

#else

template <typename T>
constexpr bool atomic_supported_size =
    sizeof(T) == 1 || sizeof(T) == 2 || sizeof(T) == 4 || sizeof(T) == 8;

template <>
inline constexpr bool atomic_supported_size<i128> = true;

template <>
inline constexpr bool atomic_supported_size<u128> = true;

template <typename T>
using enable_atomic_t = std::enable_if_t<
    std::is_trivially_copyable_v<T> && atomic_supported_size<T>, int>;

template <typename T, enable_atomic_t<T> = 0>
void atomic_store(T* ptr, T value) {
    __atomic_store_n(ptr, value, __ATOMIC_SEQ_CST);
}

template <typename T, enable_atomic_t<T> = 0>
T atomic_load(T* ptr) {
    return __atomic_load_n(ptr, __ATOMIC_SEQ_CST);
}

template <typename T, enable_atomic_t<T> = 0>
T atomic_exchange(T* ptr, T value) {
    return __atomic_exchange_n(ptr, value, __ATOMIC_SEQ_CST);
}

template <typename T, enable_atomic_t<T> = 0>
bool atomic_compare_exchange_weak(T* ptr, T& expected, T desired) {
    return __atomic_compare_exchange_n(ptr, &expected, desired,
                                       /* weak = */ true, __ATOMIC_SEQ_CST,
                                       __ATOMIC_SEQ_CST);
}

template <typename T, enable_atomic_t<T> = 0>
bool atomic_compare_exchange_strong(T* ptr, T& expected, T desired) {
    return __atomic_compare_exchange_n(ptr, &expected, desired,
                                       /* weak = */ false, __ATOMIC_SEQ_CST,
                                       __ATOMIC_SEQ_CST);
}

template <typename T, enable_atomic_t<T> = 0>
T atomic_fetch_add(T* ptr, T value) {
    return __atomic_fetch_add(ptr, value, __ATOMIC_SEQ_CST);
}

template <typename T, enable_atomic_t<T> = 0>
T atomic_fetch_sub(T* ptr, T value) {
    return __atomic_fetch_sub(ptr, value, __ATOMIC_SEQ_CST);
}

#endif

} // namespace hydra
