#pragma once

#include "ztd/type_aliases.hpp"

namespace ztd::mem {

template <typename T, usize capacity>
class StaticPool {
  public:
    StaticPool() noexcept = default;
    ~StaticPool() noexcept = default;

    ZTD_MAKE_DEFAULT_COPYABLE(StaticPool);
    ZTD_MAKE_DEFAULT_MOVABLE(StaticPool);

    template <typename... Args>
    auto insert(Args... args) noexcept
        -> std::expected<usize, IAllocator::Error> {
        ZTD_ASSIGN_OR_RETURN_ERROR(const auto index, allocateIndex());
        objects[index].emplace(std::forward<Args>(args)...);
        return index;
    }

    [[nodiscard]] auto free(usize index) noexcept -> bool {
        auto& object = objects[index];
        if (object.has_value()) {
            object = std::nullopt;
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] auto isValid(usize index) const noexcept -> bool {
        return objects[index].has_value();
    }

    auto get(usize index) noexcept -> std::optional<T>
        requires std::is_pointer_v<T>
    {
        return objects[index];
    }

    auto get(usize index) const noexcept -> std::optional<const T>
        requires std::is_pointer_v<T>
    {
        return objects[index];
    }

    auto get(usize index) noexcept -> std::optional<T*>
        requires(!std::is_pointer_v<T>)
    {
        return objects[index].transform([](T& value) -> T* { return &value; });
    }

    auto get(usize index) const noexcept -> std::optional<const T*>
        requires(!std::is_pointer_v<T>)
    {
        return objects[index].transform(
            [](const T& value) -> const T* { return &value; });
        ;
    }

    // Iterating
    auto begin() noexcept { return Iterator(this, 0); }
    auto end() noexcept { return Iterator(this, crnt); }

    auto begin() const noexcept { return ConstIterator(this, 0); }
    auto end() const noexcept { return ConstIterator(this, crnt); }

    auto cbegin() const noexcept { return begin(); }
    auto cend() const noexcept { return end(); }

    [[nodiscard]] auto getCapacity() const noexcept -> usize {
        return capacity;
    }

  private:
    std::array<std::optional<T>, capacity> objects;
    usize crnt{0};

    auto allocateIndex() noexcept -> std::expected<usize, IAllocator::Error> {
        if (crnt < capacity) {
            return crnt++;
        }

        for (usize i = 0; i < capacity; i++) {
            if (!isValid(i)) {
                return i;
            }
        }

        return std::unexpected(IAllocator::Error::OutOfMemory);
    }

    // Iterator
    template <bool is_const>
    struct IteratorBase {
        using PoolType =
            std::conditional_t<is_const, const StaticPool, StaticPool>;
        PoolType* pool;
        usize index;

        IteratorBase(PoolType* p, usize i) noexcept : pool(p), index(i) {
            while (index < pool->crnt && !pool->isValid(index)) {
                index++;
            }
        }

        auto operator*() const noexcept -> T
            requires std::is_pointer_v<T>
        {
            return *pool->objects[index];
        }
        auto operator->() const noexcept -> T
            requires std::is_pointer_v<T>
        {
            return *pool->objects[index];
        }

        auto operator*() const noexcept -> T*
            requires(!std::is_pointer_v<T>)
        {
            return &*pool->objects[index];
        }
        auto operator->() const noexcept -> T*
            requires(!std::is_pointer_v<T>)
        {
            return &*pool->objects[index];
        }

        auto operator++() noexcept -> IteratorBase& {
            do {
                index++;
            } while (index < pool->crnt && !pool->isValid(index));
            return *this;
        }

        auto operator!=(const IteratorBase& other) const noexcept -> bool {
            return index != other.index;
        }
    };

  public:
    using Iterator = IteratorBase<false>;
    using ConstIterator = IteratorBase<true>;
};

} // namespace ztd::mem
