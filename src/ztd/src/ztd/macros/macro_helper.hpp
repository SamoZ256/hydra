#pragma once

#define ZTD_CONCAT_IMPL(a, b) a##b
#define ZTD_CONCAT(a, b) ZTD_CONCAT_IMPL(a, b)

#define ZTD_PASS(...) __VA_ARGS__
#define ZTD_PASS_VA_ARGS(...) , ##__VA_ARGS__

#define ZTD_PARENS ()

#define ZTD_UNIQUE_SUFFIX(var) ZTD_CONCAT(var, __LINE__)
