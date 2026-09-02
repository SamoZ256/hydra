#pragma once

#include "ztd/mem/c_allocator.hpp"

namespace ztd::mem {

// TODO: don't use the C allocator as the default allocator
inline auto getDefaultAllocator() noexcept -> IAllocator& {
    return CAllocator::getInstance();
}

} // namespace ztd::mem
