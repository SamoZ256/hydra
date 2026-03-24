#pragma once

#include <iterator>
#include <map>
#include <string>

#include "common/functions.hpp"
#include "common/log.hpp"
#include "common/macros.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

template <typename KeyT, typename T, usize fast_cache_size = 4>
class SmallCache {
  public:
    // Iterator
    class iterator {
        friend class SmallCache;

      public:
        using map_iter = typename std::map<KeyT, T>::iterator;

        using iterator_category = std::forward_iterator_tag;
        using value_type = std::pair<const KeyT, T>;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        iterator(SmallCache* cache_, usize fast_index_)
            : cache{cache_}, fast_index{fast_index_} {
            AdvanceFast();
        }

        iterator(SmallCache* cache, map_iter slow_it)
            : cache{cache}, fast_index{fast_cache_size}, slow_it{slow_it} {}

        reference operator*() const {
            if (fast_index < fast_cache_size)
                return cache->fast_cache[fast_index].value();

            return *slow_it;
        }

        pointer operator->() const { return &(**this); }

        iterator& operator++() {
            if (fast_index < fast_cache_size) {
                ++fast_index;
                AdvanceFast();
            } else {
                ++slow_it;
            }

            return *this;
        }

        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(const iterator& other) const {
            return cache == other.cache && fast_index == other.fast_index &&
                   (fast_index < fast_cache_size || slow_it == other.slow_it);
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

      private:
        void AdvanceFast() {
            while (fast_index < fast_cache_size &&
                   !cache->fast_cache[fast_index].has_value()) {
                ++fast_index;
            }

            if (fast_index >= fast_cache_size) {
                slow_it = cache->slow_cache.begin();
            }
        }

        SmallCache* cache;
        usize fast_index;
        map_iter slow_it;
    };

    SmallCache() = default;

    SmallCache(const SmallCache&) = delete;
    SmallCache& operator=(const SmallCache&) = delete;

    SmallCache(SmallCache&&) = default;
    SmallCache& operator=(SmallCache&&) = default;

    // TODO: const versions as well
    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, slow_cache.end()); }

    // Functions
    usize GetCount() const {
        usize count = 0;
        for (auto& entry : fast_cache) {
            if (entry.has_value())
                count++;
        }

        return count + slow_cache.size();
    }

    void Clear() {
        fast_cache.fill({});
        slow_cache.clear();
    }

    enum class AddError {
        AlreadyPresent,
    };

    template <typename... Args>
    T& Add(KeyT key, Args&&... args) {
        // Insert into fast cache if possible
        for (auto& entry : fast_cache) {
            if (!entry.has_value()) {
                entry.emplace(
                    std::piecewise_construct, std::forward_as_tuple(key),
                    std::forward_as_tuple(std::forward<Args>(args)...));
                return entry.value().second;
            } else {
                ASSERT_THROWING(entry.value().first != key, Common,
                                AddError::AlreadyPresent,
                                "Entry already present");
            }
        }

        // Fallback to slow cache
        auto res = slow_cache.emplace(
            std::piecewise_construct, std::forward_as_tuple(key),
            std::forward_as_tuple(std::forward<Args>(args)...));
        ASSERT_THROWING(res.second, Common, AddError::AlreadyPresent,
                        "Entry already present");
        return res.first->second;
    }

    iterator Remove(iterator it) {
        // Fast cache
        if (it.fast_index < fast_cache_size) {
            fast_cache[it.fast_index] = std::nullopt;

            // Advance to the next element
            iterator next = it;
            ++next;
            return next;
        }

        // Slow cache
        if (it.slow_it != slow_cache.end()) {
            auto next_slow = std::next(it.slow_it);
            slow_cache.erase(it.slow_it);
            return iterator(this, next_slow);
        }

        return end();
    }

    void Remove(KeyT key) { Remove(FindIter(key)); }

    iterator FindIter(KeyT key) {
        // Fast cache
        for (u32 i = 0; i < fast_cache_size; i++) {
            if (fast_cache[i].has_value() && fast_cache[i].value().first == key)
                return iterator(this, i);
        }

        // Slow cache
        auto it = slow_cache.find(key);
        if (it != slow_cache.end()) {
            return iterator(this, it);
        }

        return end();
    }

    std::optional<T*> Find(KeyT key) {
        const auto it = FindIter(key);
        if (it == end())
            return std::nullopt;

        return &it->second;
    }

    T& FindOrAdd(KeyT key) {
        const auto opt = Find(key);
        if (opt.has_value())
            return **opt;

        return Add(key);
    }

  private:
    using FastCacheEntry = std::optional<std::pair<const KeyT, T>>;

    std::array<FastCacheEntry, fast_cache_size> fast_cache;
    std::map<KeyT, T> slow_cache;
};

} // namespace hydra
