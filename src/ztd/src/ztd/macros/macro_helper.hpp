#pragma once

#define CONCAT_IMPL(a, b) a##b
#define CONCAT(a, b) CONCAT_IMPL(a, b)

#define PASS(...) __VA_ARGS__
#define PASS_VA_ARGS(...) , ##__VA_ARGS__

#define UNIQUE_SUFFIX(var) CONCAT(var, __LINE__)
