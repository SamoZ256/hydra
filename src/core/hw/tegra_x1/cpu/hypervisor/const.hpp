#pragma once

#include <Hypervisor/Hypervisor.h>

#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

constexpr usize APPLE_PAGE_SIZE = 0x4000;

constexpr u64 PAGE_TABLE_RESERVED_SIZE = 0x01000000;
constexpr paddr_t KERNEL_PAGE_TABLE_REGION_BASE = 0x100000000;
constexpr paddr_t USER_PAGE_TABLE_REGION_BASE =
    KERNEL_PAGE_TABLE_REGION_BASE + PAGE_TABLE_RESERVED_SIZE;

constexpr uptr EXCEPTION_TRAMPOLINE_OFFSET = 0x800;

#define HV_ASSERT_SUCCESS(ret)                                                 \
    {                                                                          \
        auto res = hv_return_t(ret);                                           \
        ASSERT(res == HV_SUCCESS, Hypervisor, #ret " failed: 0x{:x}",          \
               u64(res));                                                      \
    }

// From Ryujinx
enum class ApFlags : u64 {
    ApShift = 6,
    PxnShift = 53,
    UxnShift = 54,

    UserExecuteKernelReadWriteExecute = (0UL << (int)ApShift),
    UserReadWriteExecuteKernelReadWrite = (1UL << (int)ApShift),
    UserExecuteKernelReadExecute = (2UL << (int)ApShift),
    UserReadExecuteKernelReadExecute = (3UL << (int)ApShift),

    UserExecuteKernelReadWrite = (1UL << (int)PxnShift) | (0UL << (int)ApShift),
    UserExecuteKernelRead = (1UL << (int)PxnShift) | (2UL << (int)ApShift),
    UserReadExecuteKernelRead = (1UL << (int)PxnShift) | (3UL << (int)ApShift),

    UserNoneKernelReadWriteExecute =
        (1UL << (int)UxnShift) | (0UL << (int)ApShift),
    UserReadWriteKernelReadWrite =
        (1UL << (int)UxnShift) | (1UL << (int)ApShift),
    UserNoneKernelReadExecute = (1UL << (int)UxnShift) | (2UL << (int)ApShift),
    UserReadKernelReadExecute = (1UL << (int)UxnShift) | (3UL << (int)ApShift),

    UserNoneKernelReadWrite =
        (1UL << (int)PxnShift) | (1UL << (int)UxnShift) | (0UL << (int)ApShift),
    UserNoneKernelRead =
        (1UL << (int)PxnShift) | (1UL << (int)UxnShift) | (2UL << (int)ApShift),
    UserReadKernelRead =
        (1UL << (int)PxnShift) | (1UL << (int)UxnShift) | (3UL << (int)ApShift),
};

inline uptr allocate_vm_memory(usize size) {
    ASSERT_ALIGNMENT(size, APPLE_PAGE_SIZE, Hypervisor, "size")

    void* ptr;
    posix_memalign(&ptr, APPLE_PAGE_SIZE, size);
    if (!ptr) {
        LOG_FATAL(Hypervisor, "Failed to allocate memory");
        return 0x0;
    }

    // Clear the memory
    memset(ptr, 0, size);

    return reinterpret_cast<uptr>(ptr);
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
