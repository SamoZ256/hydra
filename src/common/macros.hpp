#pragma once

#define APP_NAME "Hydra"

#define sizeof_array(array) (sizeof(array) / sizeof(array[0]))

#define PACKED __attribute__((packed, aligned(1)))

#define ONCE(code)                                                             \
    {                                                                          \
        static bool executed = false;                                          \
        if (!executed) {                                                       \
            code;                                                              \
            executed = true;                                                   \
        }                                                                      \
    }

#define THIS ((Subclass*)this)
#define CONST_THIS ((const Subclass*)this)

#define PASS(...) __VA_ARGS__
#define PASS_VA_ARGS(...) , ##__VA_ARGS__

#define SINGLETON_DEFINE_GET_INSTANCE(type, logging_class)                     \
    static type* g_instance = nullptr;                                         \
    type& type::GetInstance() {                                                \
        ASSERT_DEBUG(g_instance, logging_class,                                \
                     #type " hasn't been instantiated");                       \
        return *g_instance;                                                    \
    }

#define SINGLETON_SET_INSTANCE(type, logging_class)                            \
    {                                                                          \
        ASSERT(!g_instance, logging_class, #type " already exists");           \
        g_instance = this;                                                     \
    }

#define SINGLETON_UNSET_INSTANCE() g_instance = nullptr

#define BIT(n) (1u << (n))
#define BITL(n) (1ul << (n))

#define UNDERLYING(t) std::underlying_type_t<t>

#define ENABLE_ENUM_ARITHMETIC_OPERATORS(type)                                 \
    [[maybe_unused]] inline type operator+(type a, type b) {                   \
        return static_cast<type>(static_cast<UNDERLYING(type)>(a) +            \
                                 static_cast<UNDERLYING(type)>(b));            \
    }                                                                          \
    [[maybe_unused]] inline type operator-(type a, type b) {                   \
        return static_cast<type>(static_cast<UNDERLYING(type)>(a) -            \
                                 static_cast<UNDERLYING(type)>(b));            \
    }                                                                          \
    [[maybe_unused]] inline type operator*(type a, type b) {                   \
        return static_cast<type>(static_cast<UNDERLYING(type)>(a) *            \
                                 static_cast<UNDERLYING(type)>(b));            \
    }                                                                          \
    [[maybe_unused]] inline type operator/(type a, type b) {                   \
        return static_cast<type>(static_cast<UNDERLYING(type)>(a) /            \
                                 static_cast<UNDERLYING(type)>(b));            \
    }                                                                          \
    [[maybe_unused]] inline type operator++(type& x, i32) {                    \
        const auto tmp = x;                                                    \
        x = static_cast<type>(static_cast<UNDERLYING(type)>(x) + 1);           \
        return tmp;                                                            \
    }                                                                          \
    [[maybe_unused]] inline type operator--(type& x, i32) {                    \
        const auto tmp = x;                                                    \
        x = static_cast<type>(static_cast<UNDERLYING(type)>(x) - 1);           \
        return tmp;                                                            \
    }                                                                          \
    [[maybe_unused]] inline type& operator++(type& x) {                        \
        x = static_cast<type>(static_cast<UNDERLYING(type)>(x) + 1);           \
        return x;                                                              \
    }                                                                          \
    [[maybe_unused]] inline type& operator--(type& x) {                        \
        x = static_cast<type>(static_cast<UNDERLYING(type)>(x) - 1);           \
        return x;                                                              \
    }

#define ENABLE_ENUM_BITWISE_OPERATORS(type)                                    \
    [[maybe_unused]] inline type operator|(type a, type b) {                   \
        return static_cast<type>(static_cast<UNDERLYING(type)>(a) |            \
                                 static_cast<UNDERLYING(type)>(b));            \
    }                                                                          \
    [[maybe_unused]] inline type& operator|=(type& a, type b) {                \
        return a = a | b;                                                      \
    }                                                                          \
    [[maybe_unused]] inline type operator&(type a, type b) {                   \
        return static_cast<type>(static_cast<UNDERLYING(type)>(a) &            \
                                 static_cast<UNDERLYING(type)>(b));            \
    }                                                                          \
    [[maybe_unused]] inline type& operator&=(type& a, type b) {                \
        return a = a & b;                                                      \
    }                                                                          \
    [[maybe_unused]] inline type operator~(type a) {                           \
        return static_cast<type>(~static_cast<UNDERLYING(type)>(a));           \
    }                                                                          \
    [[maybe_unused]] inline bool any(type a) { return a != type::None; }

#define GETTER(member, name)                                                   \
    decltype(member) name() const { return member; }
#define REF_GETTER(member, name)                                               \
    decltype(member)& name() { return member; }
#define CONST_REF_GETTER(member, name)                                         \
    const decltype(member)& name() const { return member; }
#define CONSTEXPR_GETTER(member, name)                                         \
    constexpr decltype(member) name() const { return member; }

#define SETTER(member, name)                                                   \
    void name(const decltype(member) member##_) { member = member##_; }
#define CONST_REF_SETTER(member, name)                                         \
    void name(const decltype(member)& member##_) { member = member##_; }

#define GETTER_AND_SETTER(member, getter_name, setter_name)                    \
    GETTER(member, getter_name)                                                \
    SETTER(member, setter_name)
#define GETTER_AND_CONST_REF_SETTER(member, getter_name, setter_name)          \
    GETTER(member, getter_name)                                                \
    CONST_REF_SETTER(member, setter_name)

#define REF_GETTER_AND_SETTER(member, getter_name, setter_name)                \
    REF_GETTER(member, getter_name)                                            \
    SETTER(member, setter_name)
#define REF_GETTER_AND_CONST_REF_SETTER(member, getter_name, setter_name)      \
    REF_GETTER(member, getter_name)                                            \
    CONST_REF_SETTER(member, setter_name)

#define CONST_REF_GETTER_AND_SETTER(member, getter_name, setter_name)          \
    CONST_REF_GETTER(member, getter_name)                                      \
    SETTER(member, setter_name)
#define CONST_REF_GETTER_AND_CONST_REF_SETTER(member, getter_name,             \
                                              setter_name)                     \
    CONST_REF_GETTER(member, getter_name)                                      \
    CONST_REF_SETTER(member, setter_name)
#define CONSTEXPR_GETTER_AND_SETTER(member, getter_name, setter_name)          \
    CONSTEXPR_GETTER(member, getter_name)                                      \
    SETTER(member, setter_name)

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

#define FOR_EACH_2_1(macro, e1, e2, ...)                                       \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_2_1(macro, e1, e2, __VA_ARGS__)))
#define FOR_EACH_HELPER_2_1(macro, e1, e2, a, ...)                             \
    macro(e1, e2, a)                                                           \
        __VA_OPT__(FOR_EACH_AGAIN_2_1 PARENS(macro, e1, e2, __VA_ARGS__))
#define FOR_EACH_AGAIN_2_1() FOR_EACH_HELPER_2_1

#define FOR_EACH_2_2(macro, e1, e2, ...)                                       \
    __VA_OPT__(EXPAND(FOR_EACH_HELPER_2_2(macro, e1, e2, __VA_ARGS__)))
#define FOR_EACH_HELPER_2_2(macro, e1, e2, a1, a2, ...)                        \
    macro(e1, e2, a1, a2)                                                      \
        __VA_OPT__(FOR_EACH_AGAIN_2_2 PARENS(macro, e1, e2, __VA_ARGS__))
#define FOR_EACH_AGAIN_2_2() FOR_EACH_HELPER_2_2

#define ENUM_FORMAT_CASE(type, c, name)                                        \
    case type::c:                                                              \
        res = name;                                                            \
        break;

#define ENABLE_ENUM_FORMATTING(type, ...)                                      \
    template <>                                                                \
    struct fmt::formatter<type> : formatter<string_view> {                     \
        template <typename FormatContext>                                      \
        auto format(type value, FormatContext& ctx) const {                    \
            std::string_view res;                                              \
            switch (value) {                                                   \
                FOR_EACH_1_2(ENUM_FORMAT_CASE, type, __VA_ARGS__)              \
            default:                                                           \
                return formatter<string_view>::format(                         \
                    fmt::format("unknown ({})",                                \
                                static_cast<hydra::u64>(value)),               \
                    ctx);                                                      \
                break;                                                         \
            }                                                                  \
            return formatter<string_view>::format(res, ctx);                   \
        }                                                                      \
    };

#define ENABLE_ENUM_FORMATTING_WITH_INVALID(type, ...)                         \
    ENABLE_ENUM_FORMATTING(type, Invalid, "invalid", __VA_ARGS__)

#define STRUCT_FORMAT_CASE(member, f, name)                                    \
    fmt::format(name ": {" f "}", value.member),

#define ENABLE_STRUCT_FORMATTING(type, ...)                                    \
    template <>                                                                \
    struct fmt::formatter<type> : formatter<string_view> {                     \
        template <typename FormatContext>                                      \
        auto format(const type& value, FormatContext& ctx) const {             \
            /* TODO: make this more efficient */                               \
            std::string res = fmt::format(                                     \
                "{}", fmt::join(std::array{FOR_EACH_0_3(STRUCT_FORMAT_CASE,    \
                                                        __VA_ARGS__)},         \
                                ", "));                                        \
            return formatter<string_view>::format(std::move(res), ctx);        \
        }                                                                      \
    };

#define ENUM_CAST_CASE(type, value, n)                                         \
    if (value_str == n)                                                        \
        return type::value;

#define ENABLE_ENUM_CASTING(namespc, type, e_lower_case, ...)                  \
    namespace namespc {                                                        \
    inline type to_##e_lower_case(std::string_view value_str) {                \
        FOR_EACH_1_2(ENUM_CAST_CASE, type, __VA_ARGS__)                        \
        return type::Invalid;                                                  \
    }                                                                          \
    }

#define ENABLE_ENUM_FORMATTING_AND_CASTING(namespc, type, e_lower_case, ...)   \
    ENABLE_ENUM_FORMATTING_WITH_INVALID(namespc::type, __VA_ARGS__)            \
    ENABLE_ENUM_CASTING(namespc, type, e_lower_case, __VA_ARGS__)

#define ENUM_BIT_TEST(type, c, n)                                              \
    if (any(value & type::c)) {                                                \
        if (added)                                                             \
            name += " | ";                                                     \
        else                                                                   \
            added = true;                                                      \
        name += n;                                                             \
    }

#define ENABLE_ENUM_FLAGS_FORMATTING(type, ...)                                \
    template <>                                                                \
    struct fmt::formatter<type> : formatter<string_view> {                     \
        template <typename FormatContext>                                      \
        auto format(type value, FormatContext& ctx) const {                    \
            std::string name;                                                  \
            bool added = false;                                                \
            FOR_EACH_1_2(ENUM_BIT_TEST, type, __VA_ARGS__)                     \
            if (!added)                                                        \
                name = "none";                                                 \
            return formatter<string_view>::format(name, ctx);                  \
        }                                                                      \
    };
