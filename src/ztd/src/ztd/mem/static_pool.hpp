#pragma once

#include "ztd/mem/pool.hpp"

namespace ztd::mem {

template <typename T, usize capacity, bool allow_zero_handle = false>
class StaticPool : public Pool<StaticPool<T, capacity>, T, allow_zero_handle> {
    friend class Pool<StaticPool<T, capacity>, T, allow_zero_handle>;

  public:
    StaticPool() noexcept = default;
    ~StaticPool() noexcept = default;

    ZTD_MAKE_DEFAULT_COPYABLE(StaticPool);
    ZTD_MAKE_DEFAULT_MOVABLE(StaticPool);

    auto begin() noexcept { return Iterator(this, 0); }
    auto end() noexcept { return Iterator(this, capacity); }

    auto begin() const noexcept { return ConstIterator(this, 0); }
    auto end() const noexcept { return ConstIterator(this, capacity); }

    auto cbegin() const noexcept { return begin(); }
    auto cend() const noexcept { return end(); }

    [[nodiscard]] auto getCapacity() const noexcept -> usize {
        return capacity;
    }

  private:
    template <bool is_const>
    struct IteratorBase {
        using PoolType =
            std::conditional_t<is_const, const StaticPool, StaticPool>;
        PoolType* pool;
        usize index;

        IteratorBase(PoolType* p, usize i) noexcept : pool(p), index(i) {
            while (index < capacity && !pool->isValidByIndex(index)) {
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
            } while (index < capacity && !pool->isValidByIndex(index));
            return *this;
        }

        auto operator!=(const IteratorBase& other) const noexcept -> bool {
            return index != other.index;
        }
    };

    using Iterator = IteratorBase<false>;
    using ConstIterator = IteratorBase<true>;

    std::array<std::optional<T>, capacity> objects;
    u32 crnt{0};

    auto findFreeIndex() noexcept -> std::expected<u32, IAllocator::Error> {
        if (crnt < capacity) {
            return crnt++;
        }

        for (u32 i = 0; i < capacity; i++) {
            if (!isValidByIndex(i)) {
                return i;
            }
        }

        return std::unexpected(IAllocator::Error::OutOfMemory);
    }

    [[nodiscard]] auto freeByIndex(u32 index) noexcept -> bool {
        auto& object = objects[index];
        if (object.has_value()) {
            object = std::nullopt;
            return true;
        } else {
            return false;
        }
    }

    [[nodiscard]] auto isValidByIndex(u32 index) const noexcept -> bool {
        return objects[index].has_value();
    }

    auto getByIndex(u32 index) noexcept -> std::optional<T>& {
        return objects[index];
    }
    auto getByIndex(u32 index) const noexcept -> const std::optional<T>& {
        return objects[index];
    }
};

} // namespace ztd::mem
