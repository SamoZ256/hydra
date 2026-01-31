#pragma once

#include <map>
#include <string>

#include "common/functions.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

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

    T Get() const { return static_cast<T>(extract_bits(raw, b, count)); }

  private:
    Underlying raw;
};

template <typename T, u64 b, u64 count>
using BitField32 = BitField<u32, T, b, count>;

template <typename T, u64 b, u64 count>
using BitField64 = BitField<u64, T, b, count>;

// TODO: rework
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
            components[i] = static_cast<T>(other[i]);
    }

    bool operator==(const vec<T, component_count>& other) const {
        for (u32 i = 0; i < component_count; i++) {
            if (components[i] != other[i])
                return false;
        }

        return true;
    }

    T& operator[](u32 index) { return components[index]; }
    const T& operator[](u32 index) const { return components[index]; }

    T& x()
        requires(component_count >= 1)
    {
        return components[0];
    }
    T& y()
        requires(component_count >= 2)
    {
        return components[1];
    }
    T& z()
        requires(component_count >= 3)
    {
        return components[2];
    }
    T& w()
        requires(component_count >= 4)
    {
        return components[3];
    }

    T x() const
        requires(component_count >= 1)
    {
        return components[0];
    }
    T y() const
        requires(component_count >= 2)
    {
        return components[1];
    }
    T z() const
        requires(component_count >= 3)
    {
        return components[2];
    }
    T w() const
        requires(component_count >= 4)
    {
        return components[3];
    }

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
using UIntRect2D = Rect2D<u32, u32>;
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
    // HACK: allow casting from any integer
    constexpr strong_number_typedef(u64 value_)
        requires std::is_unsigned_v<T>
        : value{static_cast<T>(value_)} {}
    constexpr strong_number_typedef(i64 value_)
        requires std::is_signed_v<T>
        : value{static_cast<T>(value_)} {}

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
        u32 hash = THIS->Hash(descriptor);
        auto it = cache.find(hash);
        if (it == cache.end()) {
            it = cache.insert({hash, THIS->Create(descriptor)}).first;

            return it->second;
        }

        THIS->Update(it->second);

        return it->second;
    }

  private:
    std::map<u32, T> cache;
};

} // namespace hydra

template <typename T, hydra::usize alignment>
struct fmt::formatter<hydra::aligned<T, alignment>> : formatter<string_view> {
    fmt::formatter<T> value_formatter;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const hydra::aligned<T, alignment>& value,
                FormatContext& ctx) const {
        return value_formatter.format(value.Get(), ctx);
    }
};

template <typename T, hydra::u32 component_count>
struct fmt::formatter<hydra::vec<T, component_count>> : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::vec<T, component_count>& value,
                FormatContext& ctx) const {
        // TODO: optimize
        std::string str = "(";
        for (hydra::u32 i = 0; i < component_count; i++) {
            str += fmt::format("{}", value[i]);
            if (i != component_count - 1)
                str += ", ";
        }
        str += ")";
        return formatter<string_view>::format(str, ctx);
    }
};
