#pragma once

#include "common/common.hpp"

namespace Hydra::Allocators {

#define FREE_SIZE (size + 7) / 8

#define FREE_SLOT(index) free_slots[index / 8]
#define MASK(index) (1 << index % 8)

template <typename T, u32 size> class StaticPool {
  public:
    StaticPool() = default;
    ~StaticPool() = default;

    u32 AllocateForIndex() {
        if (crnt < size) {
            Take(crnt);
            return crnt++;
        }

        for (u32 i = 0; i < size; i++) {
            if (IsFree(i)) {
                Take(i);
                return i;
            }
        }

        LOG_ERROR(Common, "Free index not found");

        return UINT32_MAX;
    }

    T& Allocate() { return GetObjectRef(AllocateForIndex()); }

    void Free(u32 index) { FREE_SLOT(index) |= MASK(index); }

    bool IsFree(u32 index) const {
        bool is_free = FREE_SLOT(index) & MASK(index);
        return is_free;
    }

    // Getters
    T GetObject(u32 index) const {
        AssertIndex(index);
        return objects[index];
    }

    T& GetObjectRef(u32 index) {
        AssertIndex(index);
        return objects[index];
    }

  private:
    T objects[size];
    u8 free_slots[FREE_SIZE] = {UINT8_MAX};
    u32 crnt{0};

    void AssertIndex(u32 index) const {
        ASSERT_DEBUG(!IsFree(index), Common, "Invalid index {}", index);
    }

    void Take(u32 index) { FREE_SLOT(index) &= ~MASK(index); }
};

} // namespace Hydra::Allocators
