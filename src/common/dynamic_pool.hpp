#pragma once

#include "common/pool.hpp"

namespace hydra {

// TODO: this needs optimizations real bad
template <typename T, bool allow_zero_handle = false>
class DynamicPool : public Pool<DynamicPool<T>, T, allow_zero_handle> {
  public:
    u32 AllocateIndex_() {
        // TODO: look for a free index first

        const auto index = static_cast<u32>(objects.size());
        objects.push_back({});

        return index;
    }

    void FreeByIndex_(u32 index) {
        if (index == objects.size() - 1)
            objects.pop_back();
        else
            free_slots.push_back(index);
    }

    bool IsValidByIndex_(u32 index) const {
        if (index >= objects.size())
            return false;

        return std::find(free_slots.begin(), free_slots.end(), index) ==
               free_slots.end();
    }

    T& GetByIndex_(u32 index) { return objects[index]; }

    const T& GetByIndex_(u32 index) const { return objects[index]; }

    usize GetCapacity() const { return objects.size(); }

  private:
    std::vector<T> objects;
    std::vector<u32> free_slots;
};

} // namespace hydra
