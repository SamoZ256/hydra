#pragma once

#include <sys/mman.h>

#include "ztd/mem/alignment.hpp"
#include "ztd/mem/allocator.hpp"
#include "ztd/mem/page.hpp"

namespace ztd::mem {

class PageAllocator : public IAllocator {
  public:
    static auto getInstance() noexcept -> PageAllocator& {
        static PageAllocator g_instance;
        return g_instance;
    }

  protected:
    auto allocateImpl(usize size, usize alignment) noexcept
        -> std::optional<std::span<std::byte>> override {
        (void)alignment;
        const auto aligned_size = alignUp(size, getPageSize());
        const auto ptr = mmap(nullptr, aligned_size, PROT_READ | PROT_WRITE,
                              MAP_ANON | MAP_PRIVATE, -1, 0);
        if (ptr == MAP_FAILED) {
            return std::nullopt;
        }

        return std::span{reinterpret_cast<std::byte*>(ptr), aligned_size};
    }

    auto freeImpl(std::span<std::byte> bytes) noexcept -> void override {
        munmap(bytes.data(), bytes.size());
    }
};

} // namespace ztd::mem
