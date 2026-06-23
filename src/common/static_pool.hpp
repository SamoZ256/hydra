#pragma once

#include "common/pool.hpp"

namespace hydra {

#define FREE_SIZE (size + 7) / 8

#define FREE_SLOT(index) free_slots[index / 8]
#define MASK(index) (1 << index % 8)

template <typename T, u32 size, bool allow_zero_handle = false>
class StaticPool : public Pool<StaticPool<T, size>, T, allow_zero_handle> {
  public:
    StaticPool() {
        for (u32 i = 0; i < FREE_SIZE; i++)
            free_slots[i] = std::numeric_limits<u8>::max();
    }

    u32 AllocateIndex_() {
        if (crnt < size) {
            Take(crnt);
            return crnt++;
        }

        for (u32 i = 0; i < size; i++) {
            if (!IsValidByIndex_(i)) {
                Take(i);
                return i;
            }
        }

#ifdef HYDRA_DEBUG
        LOG_FATAL(Common, "Free index not found");
#endif

        return invalid<u32>();
    }

    void FreeByIndex_(u32 index) { FREE_SLOT(index) |= MASK(index); }

    bool IsValidByIndex_(u32 index) const {
        if (index >= crnt)
            return false;

        bool is_free = FREE_SLOT(index) & MASK(index);
        return !is_free;
    }

    T& GetByIndex_(u32 index) { return objects[index]; }

    const T& GetByIndex_(u32 index) const { return objects[index]; }

    usize GetCapacity() const { return size; }

  private:
    std::array<T, size> objects;
    std::array<u8, FREE_SIZE> free_slots;
    u32 crnt{0};

    void Take(u32 index) { FREE_SLOT(index) &= ~MASK(index); }
};

} // namespace hydra
