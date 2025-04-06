#pragma once

#define sizeof_array(array) (sizeof(array) / sizeof(array[0]))

#define THIS ((SubclassT*)this)

#define SINGLETON_DEFINE_GET_INSTANCE(type, logging_class, name)               \
    static type* g_instance = nullptr;                                         \
    type& type::GetInstance() {                                                \
        ASSERT_DEBUG(g_instance, logging_class,                                \
                     name " hasn't been instantiated");                        \
        return *g_instance;                                                    \
    }

#define SINGLETON_SET_INSTANCE(logging_class, name)                            \
    ASSERT(!g_instance, logging_class, name " already exists");                \
    g_instance = this;

#define SINGLETON_UNSET_INSTANCE() g_instance = nullptr

#define BIT(n) (1u << (n))
#define BITL(n) (1ul << (n))

#define ENABLE_ENUM_BITMASK_OPERATORS(e)                                       \
    inline e operator|(e a, e b) {                                             \
        return static_cast<e>(static_cast<u32>(a) | static_cast<u32>(b));      \
    }                                                                          \
    inline e& operator|=(e& a, e b) { return a = a | b; }                      \
    inline e operator&(e a, e b) {                                             \
        return static_cast<e>(static_cast<u32>(a) & static_cast<u32>(b));      \
    }                                                                          \
    inline e& operator&=(e& a, e b) { return a = a & b; }                      \
    inline e operator~(e a) { return static_cast<e>(~static_cast<u32>(a)); }   \
    inline bool any(e a) { return a != e::None; }

#define PARENS ()

#define EXPAND(...) EXPAND4(EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__))))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__))))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__))))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__))))
#define EXPAND1(...) __VA_ARGS__

#define FOR_EACH_0_1(macro, ...)                                               \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_0_1(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER_0_1(macro, a, ...)                                     \
    macro(a) __VA_OPT__(FOR_EACH_AGAIN_0_1 PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN_0_1() FOR_EACH_HELPER_0_1

#define FOR_EACH_0_2(macro, ...)                                               \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_0_2(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER_0_2(macro, a1, a2, ...)                                \
    macro(a1, a2) __VA_OPT__(FOR_EACH_AGAIN_0_2 PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN_0_2() FOR_EACH_HELPER_0_2

#define FOR_EACH_0_3(macro, ...)                                               \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_0_3(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER_0_3(macro, a1, a2, a3, ...)                            \
    macro(a1, a2, a3) __VA_OPT__(FOR_EACH_AGAIN_0_3 PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN_0_3() FOR_EACH_HELPER_0_3

#define FOR_EACH_0_4(macro, ...)                                               \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_0_4(macro, __VA_ARGS__)))
#define FOR_EACH_HELPER_0_4(macro, a1, a2, a3, a4, ...)                        \
    macro(a1, a2, a3, a4)                                                      \
        __VA_OPT__(FOR_EACH_AGAIN_0_4 PARENS(macro, __VA_ARGS__))
#define FOR_EACH_AGAIN_0_4() FOR_EACH_HELPER_0_4

#define FOR_EACH_1_2(macro, e, ...)                                            \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_1_2(macro, e, __VA_ARGS__)))
#define FOR_EACH_HELPER_1_2(macro, e, a1, a2, ...)                             \
    macro(e, a1, a2)                                                           \
        __VA_OPT__(FOR_EACH_AGAIN_1_2 PARENS(macro, e, __VA_ARGS__))
#define FOR_EACH_AGAIN_1_2() FOR_EACH_HELPER_1_2

#define FOR_EACH_1_3(macro, e, ...)                                            \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_1_3(macro, e, __VA_ARGS__)))
#define FOR_EACH_HELPER_1_3(macro, e, a1, a2, a3, ...)                         \
    macro(e, a1, a2, a3)                                                       \
        __VA_OPT__(FOR_EACH_AGAIN_1_3 PARENS(macro, e, __VA_ARGS__))
#define FOR_EACH_AGAIN_1_3() FOR_EACH_HELPER_1_3

#define ENUM_CASE(e, value, n)                                                 \
    case e::value:                                                             \
        name = n;                                                              \
        break;

#define ENABLE_ENUM_FORMATTING(e, ...)                                         \
    template <> struct fmt::formatter<e> : formatter<string_view> {            \
        template <typename FormatContext>                                      \
        auto format(e c, FormatContext& ctx) const {                           \
            string_view name;                                                  \
            switch (c) {                                                       \
                FOR_EACH_1_2(ENUM_CASE, e, __VA_ARGS__)                        \
            default:                                                           \
                name = "unknown";                                              \
                break;                                                         \
            }                                                                  \
            return formatter<string_view>::format(name, ctx);                  \
        }                                                                      \
    };

#define ENUM_BIT_TEST(e, value, n)                                             \
    if (any(c & e::value)) {                                                   \
        if (added)                                                             \
            name += " | ";                                                     \
        else                                                                   \
            added = true;                                                      \
        name += n;                                                             \
    }

#define ENABLE_ENUM_FLAGS_FORMATTING(e, ...)                                   \
    template <> struct fmt::formatter<e> : formatter<string_view> {            \
        template <typename FormatContext>                                      \
        auto format(e c, FormatContext& ctx) const {                           \
            std::string name;                                                  \
            bool added = false;                                                \
            FOR_EACH_1_2(ENUM_BIT_TEST, e, __VA_ARGS__)                        \
            if (!added)                                                        \
                name = "none";                                                 \
            return formatter<string_view>::format(name, ctx);                  \
        }                                                                      \
    };
