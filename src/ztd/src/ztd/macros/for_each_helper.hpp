#pragma once

#include "ztd/macros/macro_helper.hpp"

#define ZTD_EXPAND(...)                                                        \
    ZTD_EXPAND4(ZTD_EXPAND4(ZTD_EXPAND4(ZTD_EXPAND4(__VA_ARGS__))))
#define ZTD_EXPAND4(...)                                                       \
    ZTD_EXPAND3(ZTD_EXPAND3(ZTD_EXPAND3(ZTD_EXPAND3(__VA_ARGS__))))
#define ZTD_EXPAND3(...)                                                       \
    ZTD_EXPAND2(ZTD_EXPAND2(ZTD_EXPAND2(ZTD_EXPAND2(__VA_ARGS__))))
#define ZTD_EXPAND2(...)                                                       \
    ZTD_EXPAND1(ZTD_EXPAND1(ZTD_EXPAND1(ZTD_EXPAND1(__VA_ARGS__))))
#define ZTD_EXPAND1(...) __VA_ARGS__

#define ZTD_FOR_EACH_0_1(macro, ...)                                           \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_0_1(macro, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_0_1(macro, a, ...)                                 \
    macro(a) __VA_OPT__(ZTD_FOR_EACH_AGAIN_0_1 ZTD_PARENS(macro, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_0_1() ZTD_FOR_EACH_HELPER_0_1

#define ZTD_FOR_EACH_0_2(macro, ...)                                           \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_0_2(macro, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_0_2(macro, a1, a2, ...)                            \
    macro(a1, a2)                                                              \
        __VA_OPT__(ZTD_FOR_EACH_AGAIN_0_2 ZTD_PARENS(macro, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_0_2() ZTD_FOR_EACH_HELPER_0_2

#define ZTD_FOR_EACH_0_3(macro, ...)                                           \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_0_3(macro, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_0_3(macro, a1, a2, a3, ...)                        \
    macro(a1, a2, a3)                                                          \
        __VA_OPT__(ZTD_FOR_EACH_AGAIN_0_3 ZTD_PARENS(macro, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_0_3() ZTD_FOR_EACH_HELPER_0_3

#define ZTD_FOR_EACH_0_4(macro, ...)                                           \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_0_4(macro, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_0_4(macro, a1, a2, a3, a4, ...)                    \
    macro(a1, a2, a3, a4)                                                      \
        __VA_OPT__(ZTD_FOR_EACH_AGAIN_0_4 ZTD_PARENS(macro, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_0_4() ZTD_FOR_EACH_HELPER_0_4

#define ZTD_FOR_EACH_1_2(macro, e, ...)                                        \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_1_2(macro, e, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_1_2(macro, e, a1, a2, ...)                         \
    macro(e, a1, a2)                                                           \
        __VA_OPT__(ZTD_FOR_EACH_AGAIN_1_2 ZTD_PARENS(macro, e, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_1_2() ZTD_FOR_EACH_HELPER_1_2

#define ZTD_FOR_EACH_1_3(macro, e, ...)                                        \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_1_3(macro, e, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_1_3(macro, e, a1, a2, a3, ...)                     \
    macro(e, a1, a2, a3)                                                       \
        __VA_OPT__(ZTD_FOR_EACH_AGAIN_1_3 ZTD_PARENS(macro, e, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_1_3() ZTD_FOR_EACH_HELPER_1_3

#define ZTD_FOR_EACH_2_1(macro, e1, e2, ...)                                   \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_2_1(macro, e1, e2, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_2_1(macro, e1, e2, a, ...)                         \
    macro(e1, e2, a) __VA_OPT__(                                               \
        ZTD_FOR_EACH_AGAIN_2_1 ZTD_PARENS(macro, e1, e2, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_2_1() ZTD_FOR_EACH_HELPER_2_1

#define ZTD_FOR_EACH_2_2(macro, e1, e2, ...)                                   \
    __VA_OPT__(ZTD_EXPAND(ZTD_FOR_EACH_HELPER_2_2(macro, e1, e2, __VA_ARGS__)))
#define ZTD_FOR_EACH_HELPER_2_2(macro, e1, e2, a1, a2, ...)                    \
    macro(e1, e2, a1, a2) __VA_OPT__(                                          \
        ZTD_FOR_EACH_AGAIN_2_2 ZTD_PARENS(macro, e1, e2, __VA_ARGS__))
#define ZTD_FOR_EACH_AGAIN_2_2() ZTD_FOR_EACH_HELPER_2_2
