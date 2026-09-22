#pragma once

namespace hydra::horizon {

template <typename T, typename Pool>
class HandlePool {
  public:
    using Iterator = Pool::Iterator;
    using ConstIterator = Pool::ConstIterator;

    HandlePool() noexcept = default;
    ~HandlePool() noexcept = default;

    ZTD_MAKE_DEFAULT_COPYABLE(HandlePool);
    ZTD_MAKE_DEFAULT_MOVABLE(HandlePool);

    template <typename... Args>
    auto insert(Args... args) noexcept
        -> std::expected<Handle, ztd::mem::IAllocator::Error> {
        return pool.insert(std::forward<Args>(args)...)
            .transform(
                [](usize index) -> Handle { return Handle::fromIndex(index); });
    }

    [[nodiscard]] auto free(Handle handle) noexcept -> bool {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handle.toIndex(), false);
        return pool.free(index);
    }

    [[nodiscard]] auto isValid(Handle handle) const noexcept -> bool {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handle.toIndex(), false);
        return pool.isValid(index);
    }

    auto get(Handle handle) noexcept -> std::optional<T>
        requires std::is_pointer_v<T>
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handle.toIndex(),
                                   std::nullopt);
        return pool.get(index);
    }

    auto get(Handle handle) const noexcept -> std::optional<const T>
        requires std::is_pointer_v<T>
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handle.toIndex(),
                                   std::nullopt);
        return pool.get(index);
    }

    auto get(Handle handle) noexcept -> std::optional<T*>
        requires(!std::is_pointer_v<T>)
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handle.toIndex(),
                                   std::nullopt);
        return pool.get(index);
    }

    auto get(Handle handle) const noexcept -> std::optional<const T*>
        requires(!std::is_pointer_v<T>)
    {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto index, handle.toIndex(),
                                   std::nullopt);
        return pool.get(index);
    }

    // Iterating
    auto begin() noexcept { return pool.begin(); }
    auto end() noexcept { return pool.end(); }

    auto begin() const noexcept { return pool.begin(); }
    auto end() const noexcept { return pool.end(); }

    auto cbegin() const noexcept { return pool.cbegin(); }
    auto cend() const noexcept { return pool.cend(); }

  private:
    Pool pool;
};

template <typename T, usize capacity>
using StaticHandlePool = HandlePool<T, ztd::mem::StaticPool<T, capacity>>;

} // namespace hydra::horizon
