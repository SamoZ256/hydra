#pragma once

#include "ztd/macros/crtp_helper.hpp"
#include "ztd/type_aliases.hpp"

namespace ztd::mem {

template <typename Derived, typename T, bool allow_zero_handle>
class Pool {
  public:
    template <typename... Args>
    auto insert(Args... args) noexcept
        -> std::expected<u32, IAllocator::Error> {
        ZTD_ASSIGN_OR_RETURN_ERROR(const auto index, getSelf().findFreeIndex());
        getSelf().getByIndex(index).emplace(std::forward<Args>(args)...);
        return indexToHandle(index);
    }

    [[nodiscard]] auto free(u32 handle) noexcept -> bool {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handleToIndex(handle),
                                   false);
        return getSelf().freeByIndex(index);
    }

    auto get(u32 handle) noexcept -> std::optional<T>
        requires std::is_pointer_v<T>
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handleToIndex(handle),
                                   std::nullopt);
        return getSelf().getByIndex(index);
    }

    auto get(u32 handle) const noexcept -> std::optional<const T>
        requires std::is_pointer_v<T>
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handleToIndex(handle),
                                   std::nullopt);
        return getSelf().getByIndex(index);
    }

    auto get(u32 handle) noexcept -> std::optional<T*>
        requires(!std::is_pointer_v<T>)
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handleToIndex(handle),
                                   std::nullopt);
        return getSelf().getByIndex(index).transform(
            [](T& value) -> auto { return &value; });
    }

    auto get(u32 handle) const noexcept -> std::optional<const T*>
        requires(!std::is_pointer_v<T>)
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handleToIndex(handle),
                                   std::nullopt);
        return getSelf().getByIndex(index).transform(
            [](const T& value) -> auto { return &value; });
    }

  private:
    ZTD_DEFINE_CRTP_GET_SELF();

    // Helpers
    static auto indexToHandle(u32 index) noexcept -> u32 {
        if constexpr (allow_zero_handle)
            return index;
        else
            return index + 1;
    }

    static auto handleToIndex(u32 handle) noexcept -> std::optional<u32> {
        if constexpr (allow_zero_handle) {
            return handle;
        } else {
            if (handle == 0) {
                return std::nullopt;
            }

            return handle - 1;
        }
    }
};

} // namespace ztd::mem
