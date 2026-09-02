#pragma once

#include <bit>
#include <type_traits>

#include "ztd/type_aliases.hpp"

namespace ztd::hash {

class XxHash32 {
  public:
    void add(u32 value) noexcept {
        u32 previous_length = length++;
        u32 position = previous_length % 4;

        if (position == 0) {
            queue1 = value;
        } else if (position == 1) {
            queue2 = value;
        } else if (position == 2) {
            queue3 = value;
        } else {
            v1 = round(v1, queue1);
            v2 = round(v2, queue2);
            v3 = round(v3, queue3);
            v4 = round(v4, value);
        }
    }

    void add(u64 value) noexcept {
        u32 lower = static_cast<u32>(value);
        u32 upper = static_cast<u32>(value >> 32);
        add(lower);
        add(upper);
    }

    template <typename T>
    void add(T* ptr) noexcept {
        uptr value = reinterpret_cast<uptr>(ptr);
        if constexpr (sizeof(uptr) == 8)
            add(static_cast<u64>(value));
        else
            add(static_cast<u32>(value));
    }

    template <typename T>
    void add(const T& value) noexcept
        requires std::is_trivially_copyable_v<T>
    {
        const u8* bytes = reinterpret_cast<const u8*>(&value);
        for (usize i = 0; i < sizeof(T); ++i)
            add(static_cast<u32>(bytes[i]));
    }

    [[nodiscard]] auto toHashCode() const noexcept -> u32 {
        u32 hash = length < 4 ? mixEmptyState() : mixState(v1, v2, v3, v4);
        hash += length * 4;

        u32 position = length % 4;
        if (position > 0) {
            hash = queueRound(hash, queue1);
            if (position > 1) {
                hash = queueRound(hash, queue2);
                if (position > 2) {
                    hash = queueRound(hash, queue3);
                }
            }
        }

        return mixFinal(hash);
    }

  private:
    static constexpr u32 prime1 = 2654435761u;
    static constexpr u32 prime2 = 2246822519u;
    static constexpr u32 prime3 = 3266489917u;
    static constexpr u32 prime4 = 668265263u;
    static constexpr u32 prime5 = 374761393u;

    u32 v1{prime1 + prime2}, v2{prime2}, v3{0}, v4{prime1};
    u32 queue1{0}, queue2{0}, queue3{0};
    u32 length{0};

    static auto round(u32 hash, u32 input) noexcept -> u32 {
        return std::rotl(hash + input * prime2, 13) * prime1;
    }

    static auto queueRound(u32 hash, u32 queued_value) noexcept -> u32 {
        return std::rotl(hash + queued_value * prime3, 17) * prime4;
    }

    static auto mixState(u32 v1, u32 v2, u32 v3, u32 v4) noexcept -> u32 {
        return std::rotl(v1, 1) + std::rotl(v2, 7) + std::rotl(v3, 12) +
               std::rotl(v4, 18);
    }

    static auto mixEmptyState() noexcept -> u32 { return prime5; }

    static auto mixFinal(u32 hash) noexcept -> u32 {
        hash ^= hash >> 15;
        hash *= prime2;
        hash ^= hash >> 13;
        hash *= prime3;
        hash ^= hash >> 16;
        return hash;
    }
};

} // namespace ztd::hash
