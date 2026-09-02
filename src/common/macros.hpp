#pragma once

#define APP_NAME "Hydra"

#define sizeof_array(array) (sizeof(array) / sizeof(array[0]))

#define ONCE(code)                                                             \
    {                                                                          \
        static bool executed = false;                                          \
        if (!executed) {                                                       \
            code;                                                              \
            executed = true;                                                   \
        }                                                                      \
    }

#define THIS reinterpret_cast<Subclass*>(this)
#define CONST_THIS reinterpret_cast<const Subclass*>(this)

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
