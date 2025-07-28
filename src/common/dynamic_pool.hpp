#pragma once

#include "common/log.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

// TODO: this needs optimizations real bad
template <typename T>
class DynamicPool {
  public:
    DynamicPool() = default;
    ~DynamicPool() = default;

    u32 AllocateForIndex() {
        // TODO: look for a free index first

        u32 index = objects.size();
        objects.push_back({});

        return index;
    }

    T& Allocate() { return GetRef(AllocateForIndex()); }
    u32 Add(const T& object) {
        u32 index = AllocateForIndex();
        GetRef(index) = object;
        return index;
    }

    void Free(u32 index) {
        if (index == objects.size() - 1)
            objects.pop_back();
        else
            free_slots.push_back(index);
    }

    bool IsFree(u32 index) const {
        if (index >= objects.size())
            return true;

        return std::find(free_slots.begin(), free_slots.end(), index) !=
               free_slots.end();
    }

    T Get(u32 index) const {
        AssertIndex(index);
        return objects[index];
    }

    T GetOrDefault(u32 index) const {
        if (IsFree(index))
            return {};

        return objects[index];
    }

    T& GetRef(u32 index) {
        AssertIndex(index);
        return objects[index];
    }

    usize GetCapacity() const { return objects.size(); }

  private:
    std::vector<T> objects;
    std::vector<u32> free_slots;

    void AssertIndex(u32 index) const {
        ASSERT_DEBUG(!IsFree(index), Common, "Invalid index {}", index);
    }
};

} // namespace hydra
