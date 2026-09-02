#pragma once

#define ZTD_BIT(n) (1u << (n))
#define ZTD_BITL(n) (1ul << (n))

#define ZTD_ENABLE_ENUM_ARITHMETIC_OPERATORS(type)                             \
    [[maybe_unused]] [[nodiscard]] constexpr auto operator+(                   \
        type a, type b) noexcept -> type {                                     \
        return static_cast<type>(                                              \
            static_cast<std::underlying_type_t<type>>(a) +                     \
            static_cast<std::underlying_type_t<type>>(b));                     \
    }                                                                          \
    [[maybe_unused]] [[nodiscard]] constexpr auto operator-(                   \
        type a, type b) noexcept -> type {                                     \
        return static_cast<type>(                                              \
            static_cast<std::underlying_type_t<type>>(a) -                     \
            static_cast<std::underlying_type_t<type>>(b));                     \
    }                                                                          \
    [[maybe_unused]] constexpr auto operator++(type& x, i32) noexcept          \
        -> type {                                                              \
        const auto tmp = x;                                                    \
        x = static_cast<type>(static_cast<std::underlying_type_t<type>>(x) +   \
                              1);                                              \
        return tmp;                                                            \
    }                                                                          \
    [[maybe_unused]] constexpr auto operator--(type& x, i32) noexcept          \
        -> type {                                                              \
        const auto tmp = x;                                                    \
        x = static_cast<type>(static_cast<std::underlying_type_t<type>>(x) -   \
                              1);                                              \
        return tmp;                                                            \
    }                                                                          \
    [[maybe_unused]] constexpr auto operator++(type& x) noexcept -> type& {    \
        x = static_cast<type>(static_cast<std::underlying_type_t<type>>(x) +   \
                              1);                                              \
        return x;                                                              \
    }                                                                          \
    [[maybe_unused]] constexpr auto operator--(type& x) noexcept -> type& {    \
        x = static_cast<type>(static_cast<std::underlying_type_t<type>>(x) -   \
                              1);                                              \
        return x;                                                              \
    }

#define ZTD_ENABLE_ENUM_BITWISE_OPERATORS(type)                                \
    [[maybe_unused]] [[nodiscard]] constexpr auto operator|(                   \
        type a, type b) noexcept -> type {                                     \
        return static_cast<type>(                                              \
            static_cast<std::underlying_type_t<type>>(a) |                     \
            static_cast<std::underlying_type_t<type>>(b));                     \
    }                                                                          \
    [[maybe_unused]] constexpr auto operator|=(type& a, type b) noexcept       \
        -> type& {                                                             \
        return a = a | b;                                                      \
    }                                                                          \
    [[maybe_unused]] [[nodiscard]] constexpr auto operator&(                   \
        type a, type b) noexcept -> type {                                     \
        return static_cast<type>(                                              \
            static_cast<std::underlying_type_t<type>>(a) &                     \
            static_cast<std::underlying_type_t<type>>(b));                     \
    }                                                                          \
    [[maybe_unused]] constexpr auto operator&=(type& a, type b) noexcept       \
        -> type& {                                                             \
        return a = a & b;                                                      \
    }                                                                          \
    [[maybe_unused]] [[nodiscard]] constexpr auto operator~(type a) noexcept   \
        -> type {                                                              \
        return static_cast<type>(                                              \
            ~static_cast<std::underlying_type_t<type>>(a));                    \
    }                                                                          \
    [[maybe_unused]] [[nodiscard]] constexpr auto any(type a) noexcept         \
        -> bool {                                                              \
        return a != type::None;                                                \
    }
