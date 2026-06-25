#pragma once

#define BIT(n) (1u << (n))
#define BITL(n) (1ul << (n))

#define ENABLE_ENUM_BITWISE_OPERATORS(type)                                    \
    [[maybe_unused]] inline type operator|(type a, type b) {                   \
        return static_cast<type>(                                              \
            static_cast<std::underlying_type_t<type>>(a) |                     \
            static_cast<std::underlying_type_t<type>>(b));                     \
    }                                                                          \
    [[maybe_unused]] inline type& operator|=(type& a, type b) {                \
        return a = a | b;                                                      \
    }                                                                          \
    [[maybe_unused]] inline type operator&(type a, type b) {                   \
        return static_cast<type>(                                              \
            static_cast<std::underlying_type_t<type>>(a) &                     \
            static_cast<std::underlying_type_t<type>>(b));                     \
    }                                                                          \
    [[maybe_unused]] inline type& operator&=(type& a, type b) {                \
        return a = a & b;                                                      \
    }                                                                          \
    [[maybe_unused]] inline type operator~(type a) {                           \
        return static_cast<type>(                                              \
            ~static_cast<std::underlying_type_t<type>>(a));                    \
    }                                                                          \
    [[maybe_unused]] inline bool any(type a) { return a != type::None; }
