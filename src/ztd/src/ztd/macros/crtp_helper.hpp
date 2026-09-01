#pragma once

#define ZTD_DEFINE_CRTP_GET_SELF()                                             \
    auto getSelf() noexcept -> Derived& {                                      \
        return *static_cast<Derived*>(this);                                   \
    }                                                                          \
    auto getSelf() const noexcept -> const Derived& {                          \
        return *static_cast<const Derived*>(this);                             \
    }
