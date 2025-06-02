#pragma once

#include "core/horizon/kernel/const.hpp"

namespace hydra::horizon::kernel {

template <typename T, typename Pool> class HandlePool {
    static_assert(std::is_convertible_v<T*, Handle*>,
                  "Type does not inherit from Handle");

  public:
    handle_id_t Add(T* handle) {
        u32 index = pool.AllocateForIndex();
        pool.GetRef(index) = handle;
        return IndexToHandleID(index);
    }

    void Free(handle_id_t handle_id) {
        u32 index = HandleIdToIndex(handle_id);
        delete pool.Get(index);
        pool.FreeByIndex(index);
    }

    T* Get(handle_id_t handle_id) const {
        return pool.Get(HandleIdToIndex(handle_id));
    }

  private:
    Pool pool;

    // Helpers
    static u32 HandleIdToIndex(handle_id_t handle_id) {
        ASSERT_DEBUG(handle_id != INVALID_HANDLE_ID, Kernel,
                     "Invalid handle ID");
        return handle_id - 1;
    }

    static handle_id_t IndexToHandleID(u32 index) { return index + 1; }
};

template <typename T, usize size>
class StaticHandlePool : public HandlePool<T, StaticPool<T*, size>> {};

template <typename T>
class DynamicHandlePool : public HandlePool<T, DynamicPool<T*>> {};

} // namespace hydra::horizon::kernel
