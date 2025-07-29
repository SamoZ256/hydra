#pragma once

#include <map>
#include <string>

#include "common/functions.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

template <typename T>
struct range {
  public:
    T begin;
    T end;

    range() : begin{0}, end{0} {}
    range(T begin_) : begin{begin_}, end{invalid<T>()} {}
    range(T begin_, T end_) : begin{begin_}, end{end_} {}

    bool operator==(const range& other) const {
        return begin == other.begin && end == other.end;
    }

    bool Contains(const T v) const { return v >= begin && v < end; }

    // Getters
    T GetBegin() const { return begin; }
    T GetEnd() const { return end; }
    T GetSize() const { return end - begin; }
};

struct sized_ptr {
  public:
    sized_ptr() : ptr{0x0}, size{0} {}
    sized_ptr(uptr ptr_, usize size_) : ptr{ptr_}, size{size_} {}
    sized_ptr(void* ptr_, usize size_)
        : sized_ptr(reinterpret_cast<uptr>(ptr_), size_) {}
    template <typename T>
    sized_ptr(T* ptr_) : sized_ptr(reinterpret_cast<uptr>(ptr_), sizeof(T)) {}

    // Getters
    uptr GetPtr() const { return ptr; }
    u8* GetPtrU8() const { return reinterpret_cast<u8*>(ptr); }
    usize GetSize() const { return size; }

  private:
    uptr ptr;
    usize size;
};

template <typename T, usize component_count>
class vec {
  public:
    vec() = default;
    vec(const T& value) {
        for (usize i = 0; i < component_count; i++) {
            components[i] = value;
        }
    }
    vec(const std::initializer_list<T>& values) {
        std::copy(values.begin(), values.end(), components.begin());
    }
    template <typename OtherT, usize other_component_count>
    vec(const vec<OtherT, other_component_count>& other) {
        for (usize i = 0; i < component_count; i++) {
            components[i] = other[i];
        }
    }

    T& operator[](i32 index) { return components[index]; }

    const T& operator[](i32 index) const { return components[index]; }

    T& x() { return components[0]; }
    T& y() { return components[1]; }
    T& z() { return components[2]; }
    T& w() { return components[3]; }

    T x() const { return components[0]; }
    T y() const { return components[1]; }
    T z() const { return components[2]; }
    T w() const { return components[3]; }

  private:
    std::array<T, component_count> components = {0};
};

template <typename T, usize component_count>
vec<T, component_count> operator*(const vec<T, component_count>& l,
                                  const vec<T, component_count>& r) {
    vec<T, component_count> result = l;
    for (usize i = 0; i < component_count; i++)
        result[i] *= r[i];

    return result;
}

template <typename T, usize component_count>
vec<T, component_count> operator/(const vec<T, component_count>& l,
                                  const vec<T, component_count>& r) {
    vec<T, component_count> result = l;
    for (usize i = 0; i < component_count; i++)
        result[i] /= r[i];

    return result;
}

using char2 = vec<i8, 2>;
using short2 = vec<i16, 2>;
using int2 = vec<i32, 2>;
using long2 = vec<i64, 2>;
using uchar2 = vec<u8, 2>;
using ushort2 = vec<u16, 2>;
using uint2 = vec<u32, 2>;
using ulong2 = vec<u64, 2>;
using usize2 = vec<usize, 2>;
using float2 = vec<float, 2>;

using char3 = vec<i8, 3>;
using short3 = vec<i16, 3>;
using int3 = vec<i32, 3>;
using long3 = vec<i64, 3>;
using uchar3 = vec<u8, 3>;
using ushort3 = vec<u16, 3>;
using uint3 = vec<u32, 3>;
using ulong3 = vec<u64, 3>;
using usize3 = vec<usize, 3>;
using float3 = vec<float, 3>;

using char4 = vec<i8, 4>;
using short4 = vec<i16, 4>;
using int4 = vec<i32, 4>;
using long4 = vec<i64, 4>;
using uchar4 = vec<u8, 4>;
using ushort4 = vec<u16, 4>;
using uint4 = vec<u32, 4>;
using ulong4 = vec<u64, 4>;
using usize4 = vec<usize, 4>;
using float4 = vec<float, 4>;

template <typename Origin, typename Size>
struct Rect2D {
    vec<Origin, 2> origin;
    vec<Size, 2> size;
};

using IntRect2D = Rect2D<i32, i32>;

template <typename T, usize alignment>
class aligned {
  public:
    static_assert(sizeof(T) <= alignment);

    aligned() {}
    aligned(const T& value_) : value{value_} {}
    void operator=(const T& new_value) { value = new_value; }

    operator T&() { return value; }
    operator const T&() const { return value; }

    // Getters
    const T& Get() const { return value; }

  private:
    T value;
    u8 _pad[alignment - sizeof(T)];
} PACKED;

template <typename T>
class strong_typedef {
  public:
    strong_typedef(const T& value_) : value{value_} {}

    void operator=(const T& new_value) { value = new_value; }

    operator T&() { return value; }
    operator const T&() const { return value; }

  private:
    T value;
};

#define STRONG_TYPEDEF(type, base_type)                                        \
    class type : public strong_typedef<base_type> {                            \
      public:                                                                  \
        using strong_typedef::strong_typedef;                                  \
    }

template <typename T>
class strong_number_typedef {
  public:
    constexpr strong_number_typedef() : value{} {}
    constexpr strong_number_typedef(const T& value_) : value{value_} {}

    void operator=(const T& new_value) { value = new_value; }
    void operator+=(const T& other) { value += other; }
    void operator-=(const T& other) { value -= other; }
    void operator*=(const T& other) { value *= other; }
    void operator/=(const T& other) { value /= other; }
    void operator%=(const T& other) { value %= other; }
    void operator&=(const T& other) { value &= other; }
    void operator|=(const T& other) { value |= other; }
    void operator^=(const T& other) { value ^= other; }
    void operator<<=(const T& other) { value <<= other; }
    void operator>>=(const T& other) { value >>= other; }

    operator T&() { return value; }
    operator const T&() const { return value; }

  private:
    T value;
};

#define STRONG_NUMBER_TYPEDEF(type, base_type)                                 \
    class type : public strong_number_typedef<base_type> {                     \
      public:                                                                  \
        using strong_number_typedef::strong_number_typedef;                    \
    }

template <typename KeyT, typename T, usize fast_cache_size = 4>
class small_cache {
  public:
    T& Find(KeyT key) {
        // Check fast cache
        for (auto& entry : fast_cache) {
            if (entry.key == key) {
                return entry.value;
            }
        }

        // Check slow cache
        auto it = slow_cache.find(key);
        if (it != slow_cache.end()) {
            return it->second;
        }

        // Not found

        // Attempt to add to fast cache
        for (auto& entry : fast_cache) {
            if (entry.key == KeyT{}) {
                entry.key = key;
                entry.value = T{};
                return entry.value;
            }
        }

        // Add to slow cache as a fallback
        slow_cache[key] = T{};

        return slow_cache[key];
    }

  private:
    struct FastCacheEntry {
        KeyT key;
        T value;
    };

    std::array<FastCacheEntry, fast_cache_size> fast_cache;
    std::map<KeyT, T> slow_cache;
};

template <typename Subclass, typename T, typename DescriptorT>
class CacheBase {
  public:
    ~CacheBase() {
        for (auto& [key, value] : cache) {
            THIS->DestroyElement(value);
        }

        THIS->Destroy();
    }

    T& Find(const DescriptorT& descriptor) {
        u64 hash = THIS->Hash(descriptor);
        auto it = cache.find(hash);
        if (it == cache.end()) {
            it = cache.insert({hash, THIS->Create(descriptor)}).first;

            return it->second;
        }

        THIS->Update(it->second);

        return it->second;
    }

  private:
    std::map<u64, T> cache;
};

} // namespace hydra

template <typename T>
struct fmt::formatter<hydra::range<T>> : formatter<string_view> {
    template <typename FormatContext>
    auto format(hydra::range<T> range, FormatContext& ctx) const {
        return formatter<string_view>::format(
            fmt::format("<{}...{})", range.begin, range.end), ctx);
    }
};
