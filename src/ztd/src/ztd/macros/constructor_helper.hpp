#pragma once

#include "ztd/macros/for_each_helper.hpp"

#define ZTD_MAKE_DEFAULT_COPYABLE(type)                                        \
    type(const type&) noexcept = default;                                      \
    auto operator=(const type&) noexcept -> type& = default;

#define ZTD_MAKE_NON_COPYABLE(type)                                            \
    type(const type&) noexcept = delete;                                       \
    auto operator=(const type&) noexcept -> type& = delete;

#define ZTD_MAKE_DEFAULT_MOVABLE(type)                                         \
    type(type&&) noexcept = default;                                           \
    auto operator=(type&&) noexcept -> type& = default;

#define ZTD_MAKE_NON_MOVABLE(type)                                             \
    type(type&&) = delete;                                                     \
    auto operator=(type&&) noexcept -> type& = delete;

#define ZTD_SWAP_CASE(member) std::swap(a.member, b.member);

#define ZTD_MAKE_MOVE_ASSIGNABLE(type, ...)                                    \
    auto operator=(type&& other) noexcept -> type& {                           \
        if (this != &other) {                                                  \
            type temp(std::move(other));                                       \
            swap(*this, temp);                                                 \
        }                                                                      \
        return *this;                                                          \
    }                                                                          \
    friend auto swap(type& a, type& b) noexcept -> void {                      \
        ZTD_FOR_EACH_0_1(ZTD_SWAP_CASE, __VA_ARGS__)                           \
    }

#define ZTD_MOVE_CASE(member, value)                                           \
    , member { value }
#define ZTD_MOVE_MEMBERS(member1, value1, ...)                                 \
    member1{value1} ZTD_FOR_EACH_0_2(ZTD_MOVE_CASE, __VA_ARGS__)

#define ZTD_PASS_TO_MAKE_MOVE_ASSIGNABLE_CASE(member, value) , member
#define ZTD_PASS_TO_MAKE_MOVE_ASSIGNABLE(member1, value1, ...)                 \
    member1 ZTD_FOR_EACH_0_2(ZTD_PASS_TO_MAKE_MOVE_ASSIGNABLE_CASE, __VA_ARGS__)

#define ZTD_MAKE_MOVABLE(type, ...)                                            \
    type(type&& other) noexcept : ZTD_MOVE_MEMBERS(__VA_ARGS__) {}             \
    ZTD_MAKE_MOVE_ASSIGNABLE(type,                                             \
                             ZTD_PASS_TO_MAKE_MOVE_ASSIGNABLE(__VA_ARGS__))
