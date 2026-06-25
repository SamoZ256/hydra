#pragma once

#include "macro_helper.hpp"

#define ASSIGN_OR(var, expected, fail_statement)                               \
    const auto UNIQUE_SUFFIX(_) = expected;                                    \
    if (!UNIQUE_SUFFIX(_).has_value())                                         \
        fail_statement;                                                        \
    var = UNIQUE_SUFFIX(_).value();

#define ASSIGN_OR_RETURN_VALUE(var, expected, ret)                             \
    ASSIGN_OR(var, expected, return ret)
#define ASSIGN_OR_RETURN(var, expected) ASSIGN_OR_RETURN_VALUE(var, expected, )
#define ASSIGN_OR_RETURN_ERROR(var, expected)                                  \
    ASSIGN_OR_RETURN_VALUE(var, expected, std::unexpected(expected.error()))

#define ASSIGN_OR_CONTINUE(var, expected, ret)                                 \
    ASSIGN_OR(var, expected, continue)

#define ASSIGN_OR_BREAK(var, expected, ret) ASSIGN_OR(var, expected, break)
