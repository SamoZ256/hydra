#pragma once

#include <cstdlib>

#include "ztd/mem/allocator.hpp"

namespace ztd::mem {

class CAllocator : public IAllocator {
  public:
    static auto getInstance() noexcept -> CAllocator& {
        static CAllocator g_instance;
        return g_instance;
    }

  protected:
    auto allocImpl(usize size, usize alignment) noexcept
        -> std::optional<std::span<std::byte>> override {
        (void)alignment;
        // NOLINTBEGIN(cppcoreguidelines-owning-memory,
        // cppcoreguidelines-no-malloc)
        const auto ptr = malloc(size);
        // NOLINTEND(cppcoreguidelines-owning-memory,
        // cppcoreguidelines-no-malloc)
        if (ptr == nullptr)
            return std::nullopt;

        return std::span{reinterpret_cast<std::byte*>(ptr), size};
    }

    auto freeImpl(std::span<std::byte> bytes) noexcept -> void override {
        // NOLINTBEGIN(cppcoreguidelines-owning-memory,
        // cppcoreguidelines-no-malloc)
        ::free(bytes.data());
        // NOLINTEND(cppcoreguidelines-owning-memory,
        // cppcoreguidelines-no-malloc)
    }
};

} // namespace ztd::mem
