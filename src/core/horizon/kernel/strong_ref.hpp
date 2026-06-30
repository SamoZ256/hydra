#pragma once

namespace hydra::horizon::kernel {

template <typename T>
class StrongRef {
    static_assert(std::is_base_of<AutoObject, T>::value,
                  "T must derive from AutoObject");

  public:
    StrongRef(T* obj_) noexcept : obj{obj_} { obj->Retain(); }

    template <typename... Args>
    StrongRef(Args&&... args) noexcept
        : obj{new T(std::forward<Args>(args)...)} {}

    ~StrongRef() { obj->Release(); }

    ZTD_MAKE_NON_COPYABLE(StrongRef);
    ZTD_MAKE_DEFAULT_MOVABLE(StrongRef);

    T* operator*() const { return obj; }
    T* operator->() const { return obj; }

  private:
    T* obj;

  public:
    GETTER(obj, Get);
};

} // namespace hydra::horizon::kernel
