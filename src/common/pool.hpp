#pragma once

#include "common/log.hpp"
#include "common/type_aliases.hpp"

namespace hydra {

template <typename Subclass, typename T, bool allow_zero_handle>
class Pool {
  public:
    enum class Error {
        InvalidHandle,
    };

    handle_id_t AllocateHandle() {
        return IndexToHandle(THIS->_AllocateIndex());
    }

    T& Allocate() { return THIS->_GetByIndex(THIS->_AllocateIndex()); }

    handle_id_t Add(const T& object) {
        const auto index = THIS->_AllocateIndex();
        THIS->_GetByIndex(index) = object;
        return IndexToHandle(index);
    }

    void Free(handle_id_t handle_id) {
        THIS->_FreeByIndex(HandleToIndex(handle_id));
    }

    bool IsValid(handle_id_t handle_id) const {
        return CONST_THIS->_IsValidByIndex(HandleToIndex(handle_id));
    }

    T& Get(handle_id_t handle_id) {
        AssertHandle(handle_id);
        return THIS->_GetByIndex(HandleToIndex(handle_id));
    }

    const T& Get(handle_id_t handle_id) const {
        AssertHandle(handle_id);
        return CONST_THIS->_GetByIndex(HandleToIndex(handle_id));
    }

  private:
    // Helpers
    void AssertHandle(handle_id_t handle_id) const {
        ASSERT_THROWING_DEBUG(IsValid(handle_id), Common, Error::InvalidHandle,
                              "Invalid handle {}", handle_id);
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
            ASSERT_THROWING_DEBUG(handle_id != INVALID_HANDLE_ID, Common,
                                  Error::InvalidHandle, "Invalid handle");
            return handle_id - 1;
        }
    }
};

} // namespace hydra
