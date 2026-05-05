#pragma once

#include "type_aliases.hpp"

namespace hydra {

class HashCode {
  public:
    HashCode()
        : v1{prime1 + prime2}, v2{prime2}, v3{0}, v4{prime1}, queue1{0},
          queue2{0}, queue3{0}, length{0} {}

    void Add(u32 value) {
        u32 previous_length = length++;
        u32 position = previous_length % 4;

        if (position == 0)
            queue1 = value;
        else if (position == 1)
            queue2 = value;
        else if (position == 2)
            queue3 = value;
        else {
            v1 = Round(v1, queue1);
            v2 = Round(v2, queue2);
            v3 = Round(v3, queue3);
            v4 = Round(v4, value);
        }
    }

    void Add(u64 value) {
        u32 lower = static_cast<u32>(value);
        u32 upper = static_cast<u32>(value >> 32);
        Add(lower);
        Add(upper);
    }

    template <typename T>
    void Add(T* ptr) {
        uptr value = reinterpret_cast<uptr>(ptr);
        if constexpr (sizeof(uptr) == 8)
            Add(static_cast<u64>(value));
        else
            Add(static_cast<u32>(value));
    }

    template <typename T>
    void Add(const T& value) requires std::is_trivially_copyable_v<T> {
        const u8* bytes = reinterpret_cast<const u8*>(&value);
        for (usize i = 0; i < sizeof(T); ++i)
            Add(static_cast<u32>(bytes[i]));
    }

    u32 ToHashCode() {
        u32 hash = length < 4 ? MixEmptyState() : MixState(v1, v2, v3, v4);
        hash += length * 4;

        u32 position = length % 4;
        if (position > 0) {
            hash = QueueRound(hash, queue1);
            if (position > 1) {
                hash = QueueRound(hash, queue2);
                if (position > 2) {
                    hash = QueueRound(hash, queue3);
                }
            }
        }

        return MixFinal(hash);
    }

  private:
    static constexpr u32 prime1 = 2654435761u;
    static constexpr u32 prime2 = 2246822519u;
    static constexpr u32 prime3 = 3266489917u;
    static constexpr u32 prime4 = 668265263u;
    static constexpr u32 prime5 = 374761393u;

    u32 v1, v2, v3, v4;
    u32 queue1, queue2, queue3;
    u32 length;

    static u32 Round(u32 hash, u32 input) {
        return std::rotl(hash + input * prime2, 13) * prime1;
    }

    static u32 QueueRound(u32 hash, u32 queued_value) {
        return std::rotl(hash + queued_value * prime3, 17) * prime4;
    }

    static u32 MixState(u32 v1, u32 v2, u32 v3, u32 v4) {
        return std::rotl(v1, 1) + std::rotl(v2, 7) + std::rotl(v3, 12) +
               std::rotl(v4, 18);
    }

    static u32 MixEmptyState() { return prime5; }

    static u32 MixFinal(u32 hash) {
        hash ^= hash >> 15;
        hash *= prime2;
        hash ^= hash >> 13;
        hash *= prime3;
        hash ^= hash >> 16;
        return hash;
    }
};

} // namespace hydra
