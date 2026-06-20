#pragma once

#include "common/log.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

template <typename Subclass, typename T, bool allow_zero_handle>
class Pool {
  public:
    handle_id_t AllocateHandle() {
        return IndexToHandle(THIS->AllocateIndex_());
    }

    T& Allocate() { return THIS->GetByIndex_(THIS->AllocateIndex_()); }

    handle_id_t Insert(const T& object) {
        const auto index = THIS->AllocateIndex_();
        THIS->GetByIndex_(index) = object;
        return IndexToHandle(index);
    }

    void Free(handle_id_t handle_id) {
        THIS->FreeByIndex_(HandleToIndex(handle_id));
    }

    bool IsValid(handle_id_t handle_id) const {
        return CONST_THIS->IsValidByIndex_(HandleToIndex(handle_id));
    }

    T& Get(handle_id_t handle_id) {
        AssertHandle(handle_id);
        return THIS->GetByIndex_(HandleToIndex(handle_id));
    }

    const T& Get(handle_id_t handle_id) const {
        AssertHandle(handle_id);
        return CONST_THIS->GetByIndex_(HandleToIndex(handle_id));
    }

  private:
    // Helpers
    void AssertHandle(handle_id_t handle_id) const {
        ASSERT_DEBUG(IsValid(handle_id), Common, "Invalid handle {}",
                     handle_id);
    }

    static handle_id_t IndexToHandle(u32 index) {
        if constexpr (allow_zero_handle)
            return index;
        else
            return index + 1;
    }

    static u32 HandleToIndex(handle_id_t handle_id) {
        if constexpr (allow_zero_handle) {
            return handle_id;
        } else {
            ASSERT_DEBUG(handle_id != INVALID_HANDLE_ID, Common,
                         "Invalid handle");
            return handle_id - 1;
        }
    }
};

} // namespace hydra
