#pragma once

#include <expected>
#include <optional>

#include "ztd/macros/constructor_helper.hpp"
#include "ztd/macros/optional_helper.hpp"
#include "ztd/type_aliases.hpp"

namespace ztd::mem {

class IAllocator {
  public:
    enum class Error : u8 {
        OutOfMemory,
    };

    IAllocator() noexcept = default;
    virtual ~IAllocator() noexcept = default;

    ZTD_MAKE_DEFAULT_COPYABLE(IAllocator);
    ZTD_MAKE_DEFAULT_MOVABLE(IAllocator);

    template <typename T, typename... Args>
    auto create(Args... args) noexcept -> std::expected<T*, Error> {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto bytes,
                                   allocImpl(sizeof(T), alignof(T)),
                                   std::unexpected(Error::OutOfMemory));
        const auto ptr = reinterpret_cast<T*>(bytes.data());
        new (ptr) T(std::forward<Args>(args)...);
        return ptr;
    }

    template <typename T>
    auto alloc() noexcept -> std::expected<T*, Error> {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto bytes,
                                   allocImpl(sizeof(T), alignof(T)),
                                   std::unexpected(Error::OutOfMemory));
        return reinterpret_cast<T*>(bytes.data());
    }

    template <typename T>
    auto alloc(usize count) noexcept -> std::expected<std::span<T>, Error> {
        ZTD_ASSIGN_OR_RETURN_VALUE(const auto bytes,
                                   allocImpl(sizeof(T) * count, alignof(T)),
                                   std::unexpected(Error::OutOfMemory));
        return {reinterpret_cast<T*>(bytes.data()), count};
    }

    template <typename T>
    auto destroy(T* ptr) noexcept -> void {
        ptr->~T();
        freeImpl({reinterpret_cast<std::byte*>(ptr), sizeof(T)});
    }

    template <typename T>
    auto free(T* ptr) noexcept -> void {
        freeImpl({reinterpret_cast<std::byte*>(ptr), sizeof(T)});
    }

    template <typename T>
    auto free(std::span<T> span) noexcept -> void {
        freeImpl(
            {reinterpret_cast<std::byte*>(span.data()), span.size_bytes()});
    }

  protected:
    virtual auto allocImpl(usize size, usize alignment) noexcept
        -> std::optional<std::span<std::byte>> = 0;
    virtual auto freeImpl(std::span<std::byte> bytes) noexcept -> void = 0;
};

} // namespace ztd::mem
