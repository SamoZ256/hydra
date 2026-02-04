#pragma once

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

        iterator(SmallCache* cache_, usize fast_index_)
            : cache{cache_}, fast_index{fast_index_} {
            advance_fast();
        }

        iterator(SmallCache* cache, map_iter slow_it)
            : cache(cache), fast_index(fast_cache_size), slow_it(slow_it) {}

        std::pair<const KeyT&, T&> operator*() const {
            if (fast_index < fast_cache_size) {
                auto& e = cache->fast_cache[fast_index];
                return {e.key, e.value};
            }
            return {slow_it->first, slow_it->second};
        }

        iterator& operator++() {
            if (fast_index < fast_cache_size) {
                ++fast_index;
                advance_fast();
            } else {
                ++slow_it;
            }
            return *this;
        }

        bool operator==(const iterator& other) const {
            return cache == other.cache && fast_index == other.fast_index &&
                   (fast_index < fast_cache_size || slow_it == other.slow_it);
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

      private:
        void advance_fast() {
            while (fast_index < fast_cache_size &&
                   !cache->fast_cache[fast_index].occupied) {
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

    iterator begin() { return iterator(this, 0); }
    iterator end() { return iterator(this, slow_cache.end()); }

    void Clear() {
        fast_cache.fill({});
        slow_cache.clear();
    }

    enum class AddError {
        AlreadyPresent,
    };

    T& Add(KeyT key, const T& value = {}) {
        // Insert into fast cache if possible
        for (auto& entry : fast_cache) {
            if (!entry.occupied) {
                entry.occupied = true;
                entry.key = key;
                entry.value = value;
                return entry.value;
            } else {
                ASSERT_THROWING(entry.key != key, Common,
                                AddError::AlreadyPresent,
                                "Entry already present");
            }
        }

        // Fallback to slow cache
        auto res = slow_cache.emplace(key, value);
        ASSERT_THROWING(res.second, Common, AddError::AlreadyPresent,
                        "Entry already present");
        return res.first->second;
    }

    iterator Remove(iterator it) {
        // Fast cache
        if (it.fast_index < fast_cache_size) {
            fast_cache[it.fast_index].occupied = false;

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
            if (fast_cache[i].occupied && fast_cache[i].key == key)
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

        return &(*it).second;
    }

    T& FindOrAdd(KeyT key) {
        const auto opt = Find(key);
        if (opt.has_value())
            return **opt;

        return Add(key);
    }

  private:
    struct FastCacheEntry {
        bool occupied{false};
        KeyT key;
        T value;
    };

    std::array<FastCacheEntry, fast_cache_size> fast_cache;
    std::map<KeyT, T> slow_cache;
};

} // namespace hydra
