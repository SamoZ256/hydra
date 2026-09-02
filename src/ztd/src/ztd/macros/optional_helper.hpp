#pragma once

#include "macro_helper.hpp"

#define ZTD_ASSIGN_OR(var, expected, fail_statement)                           \
    auto ZTD_UNIQUE_SUFFIX(_) = expected;                                      \
    if (!ZTD_UNIQUE_SUFFIX(_).has_value())                                     \
        fail_statement;                                                        \
    var = std::move(ZTD_UNIQUE_SUFFIX(_).value());

#define ZTD_ASSIGN_OR_RETURN_VALUE(var, expected, ret)                         \
    ZTD_ASSIGN_OR(var, expected, return ret)
#define ZTD_ASSIGN_OR_RETURN(var, expected)                                    \
    ZTD_ASSIGN_OR_RETURN_VALUE(var, expected, )
#define ZTD_ASSIGN_OR_RETURN_ERROR(var, expected)                              \
    ZTD_ASSIGN_OR_RETURN_VALUE(var, expected, std::unexpected(expected.error()))

#define ZTD_ASSIGN_OR_CONTINUE(var, expected, ret)                             \
    ZTD_ASSIGN_OR(var, expected, continue)

#define ZTD_ASSIGN_OR_BREAK(var, expected, ret)                                \
    ZTD_ASSIGN_OR(var, expected, break)
