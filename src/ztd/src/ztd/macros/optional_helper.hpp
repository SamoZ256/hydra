#pragma once

#include "macro_helper.hpp"

#define ZTD_ASSIGN_OR(var, expected, fail_statement)                           \
    auto ZTD_UNIQUE_SUFFIX(_) = expected;                                      \
    if (!ZTD_UNIQUE_SUFFIX(_).has_value()) {                                   \
        fail_statement;                                                        \
    }                                                                          \
    var = std::move(ZTD_UNIQUE_SUFFIX(_).value());

#define ZTD_ASSIGN_WITH_ERROR_OR(var, expected, fail_statement)                \
    ZTD_ASSIGN_OR(var, expected, {                                             \
        const auto error = ZTD_UNIQUE_SUFFIX(_).error();                       \
        fail_statement;                                                        \
    })

#define ZTD_ASSIGN_OR_RETURN_VALUE(var, expected, ret)                         \
    ZTD_ASSIGN_OR(var, expected, return ret)
#define ZTD_ASSIGN_WITH_ERROR_OR_RETURN_VALUE(var, expected, ret)              \
    ZTD_ASSIGN_WITH_ERROR_OR(var, expected, return ret)
#define ZTD_ASSIGN_OR_RETURN(var, expected)                                    \
    ZTD_ASSIGN_OR_RETURN_VALUE(var, expected, )
#define ZTD_ASSIGN_OR_RETURN_ERROR(var, expected)                              \
    ZTD_ASSIGN_WITH_ERROR_OR_RETURN_VALUE(var, expected, std::unexpected(error))

#define ZTD_ASSIGN_OR_CONTINUE(var, expected)                                  \
    ZTD_ASSIGN_OR(var, expected, continue)

#define ZTD_ASSIGN_OR_BREAK(var, expected) ZTD_ASSIGN_OR(var, expected, break)
