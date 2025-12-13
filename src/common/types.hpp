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

    static constexpr range<T> FromSize(T begin, T size) {
        return range<T>(begin, begin + size);
    }

    constexpr range() : begin{0}, end{0} {}
    constexpr range(T begin_) : begin{begin_}, end{invalid<T>()} {}
    constexpr range(T begin_, T end_) : begin{begin_}, end{end_} {}

    bool operator==(const range& other) const {
        return begin == other.begin && end == other.end;
    }

    void Shift(T offset) {
        begin += offset;
        end += offset;
    }
    void ShiftLeft(T offset) {
        begin -= offset;
        end -= offset;
    }

    bool Contains(const T other) const { return other >= begin && other < end; }
    bool Contains(const range<T>& other) const {
        return other.begin >= begin && other.end <= end;
    }

    bool Intersects(const range<T>& other) const {
        return begin < other.end && end > other.begin;
    }

    T GetSize() const { return end - begin; }

  public:
    GETTER(begin, GetBegin);
    GETTER(end, GetEnd);
};

struct sized_ptr {
  public:
    sized_ptr() : ptr{0x0}, size{0} {}
    sized_ptr(uptr ptr_, usize size_) : ptr{ptr_}, size{size_} {}
    sized_ptr(void* ptr_, usize size_)
        : sized_ptr(reinterpret_cast<uptr>(ptr_), size_) {}
    template <typename T>
    sized_ptr(T* ptr_) : sized_ptr(reinterpret_cast<uptr>(ptr_), sizeof(T)) {}

    u8* GetPtrU8() const { return reinterpret_cast<u8*>(ptr); }

  private:
    uptr ptr;
    usize size;

  public:
    GETTER(ptr, GetPtr);
    GETTER(size, GetSize);
};

template <typename Underlying, typename T, u64 b, u64 count>
class BitField {
  public:
    operator T() { return Get(); }

    T Get() const {
        return static_cast<T>(extract_bits<Underlying, b, count>(raw));
    }

  private:
    Underlying raw;
};

template <typename T, u64 b, u64 count>
using BitField32 = BitField<u32, T, b, count>;

template <typename T, u64 b, u64 count>
using BitField64 = BitField<u64, T, b, count>;

template <typename T, u32 component_count>
class vec {
  public:
    vec() = default;
    vec(const T& value) {
        for (u32 i = 0; i < component_count; i++)
            components[i] = value;
    }
    vec(const std::initializer_list<T>& values) {
        std::copy(values.begin(), values.end(), components.begin());
    }
    template <typename OtherT, u32 other_component_count>
    vec(const vec<OtherT, other_component_count>& other) {
        for (u32 i = 0; i < component_count; i++)
            components[i] = other[i];
    }

    bool operator==(const vec<T, component_count>& other) const {
        for (u32 i = 0; i < component_count; i++) {
            if (components[i] != other[i])
                return false;
        }

        return true;
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

template <typename T, u32 component_count>
vec<T, component_count> operator+(const vec<T, component_count>& l, T r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] += r;

    return result;
}

template <typename T, u32 component_count>
vec<T, component_count> operator+(const vec<T, component_count>& l,
                                  const vec<T, component_count>& r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] += r[i];

    return result;
}

template <typename T, u32 component_count>
vec<T, component_count> operator-(const vec<T, component_count>& l, T r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] -= r;

    return result;
}

template <typename T, u32 component_count>
vec<T, component_count> operator-(const vec<T, component_count>& l,
                                  const vec<T, component_count>& r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] -= r[i];

    return result;
}

template <typename T, u32 component_count>
vec<T, component_count> operator*(const vec<T, component_count>& l, T r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] *= r;

    return result;
}

template <typename T, u32 component_count>
vec<T, component_count> operator*(const vec<T, component_count>& l,
                                  const vec<T, component_count>& r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] *= r[i];

    return result;
}

template <typename T, u32 component_count>
vec<T, component_count> operator/(const vec<T, component_count>& l, T r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] /= r;

    return result;
}

template <typename T, u32 component_count>
vec<T, component_count> operator/(const vec<T, component_count>& l,
                                  const vec<T, component_count>& r) {
    vec<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
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

    Rect2D() {}

    Rect2D(vec<Origin, 2> origin_, vec<Size, 2> size_)
        : origin{origin_}, size{size_} {}

    template <typename OtherOrigin, typename OtherSize>
    Rect2D(const Rect2D<OtherOrigin, OtherSize>& other)
        : origin{other.origin}, size{other.size} {}
};

using IntRect2D = Rect2D<i32, i32>;
using FloatRect2D = Rect2D<f32, f32>;

template <typename T, usize alignment>
class aligned {
  public:
    static_assert(sizeof(T) <= alignment);

    aligned() {}
    aligned(const T& value_) : value{value_} {}
    void operator=(const T& new_value) { value = new_value; }

    operator T&() { return value; }
    operator const T&() const { return value; }

    void ZeroOutPadding() { std::memset(_padding, 0, sizeof_array(_padding)); }

  private:
    T value;
    u8 _padding[alignment - sizeof(T)];

  public:
    CONST_REF_GETTER(value, Get);
} PACKED;

template <typename T>
class strong_typedef {
  public:
    strong_typedef() : value{} {}
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

template <typename T, hydra::u32 component_count>
struct fmt::formatter<hydra::vec<T, component_count>> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::vec<T, component_count>& vec,
                FormatContext& ctx) const {
        // TODO: optimize
        std::string str = "(";
        for (hydra::u32 i = 0; i < component_count; i++) {
            str += fmt::format("{}", vec[i]);
            if (i != component_count - 1)
                str += ", ";
        }
        str += ")";
        return formatter<string_view>::format(str, ctx);
    }
};
