#pragma once

#include <map>
#include <string>

#include "common/functions.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

template <typename Underlying, typename T, u64 b, u64 count>
class BitField {
  public:
    // NOLINTNEXTLINE(cppcoreguidelines-explicit-constructor)
    operator T() { return get(); }

    T get() const { return static_cast<T>(extractBits(raw, b, count)); }

  private:
    Underlying raw;
};

template <typename T, u64 b, u64 count>
using BitField32 = BitField<u32, T, b, count>;

template <typename T, u64 b, u64 count>
using BitField64 = BitField<u64, T, b, count>;

// TODO: rework
template <typename T, u32 component_count>
class Vector {
  public:
    constexpr Vector() = default;
    // NOLINTBEGIN(cppcoreguidelines-explicit-constructor)
    constexpr Vector(const T& value) {
        for (u32 i = 0; i < component_count; i++)
            components[i] = value;
    }
    constexpr Vector(const std::initializer_list<T>& values) {
        std::copy(values.begin(), values.end(), components.begin());
    }
    template <typename OtherT, u32 other_component_count>
    constexpr Vector(const Vector<OtherT, other_component_count>& other) {
        for (u32 i = 0; i < component_count; i++)
            components[i] = static_cast<T>(other[i]);
    }
    // NOLINTEND(cppcoreguidelines-explicit-constructor)

    bool operator==(const Vector<T, component_count>& other) const {
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
Vector<T, component_count> operator+(const Vector<T, component_count>& l, T r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] += r;

    return result;
}

template <typename T, u32 component_count>
Vector<T, component_count> operator+(const Vector<T, component_count>& l,
                                     const Vector<T, component_count>& r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] += r[i];

    return result;
}

template <typename T, u32 component_count>
Vector<T, component_count> operator-(const Vector<T, component_count>& l, T r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] -= r;

    return result;
}

template <typename T, u32 component_count>
Vector<T, component_count> operator-(const Vector<T, component_count>& l,
                                     const Vector<T, component_count>& r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] -= r[i];

    return result;
}

template <typename T, u32 component_count>
Vector<T, component_count> operator*(const Vector<T, component_count>& l, T r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] *= r;

    return result;
}

template <typename T, u32 component_count>
Vector<T, component_count> operator*(const Vector<T, component_count>& l,
                                     const Vector<T, component_count>& r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] *= r[i];

    return result;
}

template <typename T, u32 component_count>
Vector<T, component_count> operator/(const Vector<T, component_count>& l, T r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] /= r;

    return result;
}

template <typename T, u32 component_count>
Vector<T, component_count> operator/(const Vector<T, component_count>& l,
                                     const Vector<T, component_count>& r) {
    Vector<T, component_count> result = l;
    for (u32 i = 0; i < component_count; i++)
        result[i] /= r[i];

    return result;
}

using char2 = Vector<i8, 2>;
using short2 = Vector<i16, 2>;
using int2 = Vector<i32, 2>;
using long2 = Vector<i64, 2>;
using uchar2 = Vector<u8, 2>;
using ushort2 = Vector<u16, 2>;
using uint2 = Vector<u32, 2>;
using ulong2 = Vector<u64, 2>;
using usize2 = Vector<usize, 2>;
using float2 = Vector<float, 2>;

using char3 = Vector<i8, 3>;
using short3 = Vector<i16, 3>;
using int3 = Vector<i32, 3>;
using long3 = Vector<i64, 3>;
using uchar3 = Vector<u8, 3>;
using ushort3 = Vector<u16, 3>;
using uint3 = Vector<u32, 3>;
using ulong3 = Vector<u64, 3>;
using float3 = Vector<float, 3>;

using char4 = Vector<i8, 4>;
using short4 = Vector<i16, 4>;
using int4 = Vector<i32, 4>;
using long4 = Vector<i64, 4>;
using uchar4 = Vector<u8, 4>;
using ushort4 = Vector<u16, 4>;
using uint4 = Vector<u32, 4>;
using ulong4 = Vector<u64, 4>;
using usize4 = Vector<usize, 4>;
using float4 = Vector<float, 4>;

template <typename Origin, typename Size>
struct Rect2D {
    Vector<Origin, 2> origin;
    Vector<Size, 2> size;

    Rect2D() = default;

    Rect2D(Vector<Origin, 2> origin_, Vector<Size, 2> size_)
        : origin{origin_}, size{size_} {}

    template <typename OtherOrigin, typename OtherSize>
    // NOLINTNEXTLINE(cppcoreguidelines-explicit-constructor)
    Rect2D(const Rect2D<OtherOrigin, OtherSize>& other)
        : origin{other.origin}, size{other.size} {}
};

using IntRect2D = Rect2D<i32, i32>;
using UIntRect2D = Rect2D<u32, u32>;
using FloatRect2D = Rect2D<f32, f32>;

// TODO: handle this better
#pragma pack(push, 1)
template <typename T, usize alignment>
class Aligned {
  public:
    static_assert(sizeof(T) <= alignment);

    Aligned() = default;
    // NOLINTBEGIN(cppcoreguidelines-explicit-constructor)
    Aligned(const T& value_) : value{value_} {}
    Aligned& operator=(const T& new_value) {
        value = new_value;
        return *this;
    }

    operator T&() { return value; }
    operator const T&() const { return value; }
    // NOLINTEND(cppcoreguidelines-explicit-constructor)

    void zeroOutPadding() { std::fill(padding.begin(), padding.end(), 0); }

  private:
    T value;
    std::array<u8, alignment - sizeof(T)> padding;

  public:
    CONST_REF_GETTER(value, get);
};
#pragma pack(pop)

template <typename T>
class strong_typedef {
  public:
    strong_typedef() : value{} {}
    // NOLINTBEGIN(cppcoreguidelines-explicit-constructor)
    strong_typedef(const T& value_) : value{value_} {}

    strong_typedef<T>& operator=(const T& new_value) {
        value = new_value;
        return *this;
    }

    operator T&() { return value; }
    operator const T&() const { return value; }
    // NOLINTEND(cppcoreguidelines-explicit-constructor)

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
    // NOLINTBEGIN(cppcoreguidelines-explicit-constructor)
    // HACK: allow casting from any integer
    constexpr strong_number_typedef(u64 value_)
        requires std::is_unsigned_v<T>
        : value{static_cast<T>(value_)} {}
    constexpr strong_number_typedef(i64 value_)
        requires std::is_signed_v<T>
        : value{static_cast<T>(value_)} {}

    strong_number_typedef<T>& operator=(const T& new_value) {
        value = new_value;
        return *this;
    }
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
    // NOLINTEND(cppcoreguidelines-explicit-constructor)

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
    friend Subclass;

    CacheBase() noexcept = default;

  public:
    ~CacheBase() noexcept {
        for (auto& [key, value] : cache) {
            THIS->destroyElement(value);
        }

        THIS->destroy();
    }

    ZTD_MAKE_NON_COPYABLE(CacheBase);

    T& find(const DescriptorT& descriptor) {
        u32 hash = THIS->hash(descriptor);
        auto it = cache.find(hash);
        if (it == cache.end()) {
            it = cache.insert({hash, THIS->create(descriptor)}).first;

            return it->second;
        }

        THIS->update(it->second);

        return it->second;
    }

  private:
    std::map<u32, T> cache;
};

} // namespace hydra

template <typename T, hydra::usize alignment>
struct fmt::formatter<hydra::Aligned<T, alignment>> : formatter<string_view> {
    fmt::formatter<T> value_formatter;

    constexpr auto parse(fmt::format_parse_context& ctx) {
        return value_formatter.parse(ctx);
    }

    template <typename FormatContext>
    auto format(const hydra::Aligned<T, alignment>& value,
                FormatContext& ctx) const {
        return value_formatter.format(value.get(), ctx);
    }
};

template <typename T, hydra::u32 component_count>
struct fmt::formatter<hydra::Vector<T, component_count>>
    : formatter<string_view> {
    template <typename FormatContext>
    auto format(const hydra::Vector<T, component_count>& value,
                FormatContext& ctx) const {
        // TODO: optimize
        std::string str = "(";
        for (hydra::u32 i = 0; i < component_count; i++) {
            str += fmt::format("{}", value[i]);
            if (i != component_count - 1)
                str += ", ";
        }
        str += ')';
        return formatter<string_view>::format(str, ctx);
    }
};
