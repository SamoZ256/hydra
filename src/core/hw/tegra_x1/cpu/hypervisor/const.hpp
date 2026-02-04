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

constexpr u32 AP_SHIFT = 6;
constexpr u32 PNX_SHIFT = 53;
constexpr u32 UXN_SHIFT = 54;

// From Ryujinx
enum class ApFlags : u64 {
    UserExecuteKernelReadWriteExecute = (0ull << AP_SHIFT),
    UserReadWriteExecuteKernelReadWrite = (1ull << AP_SHIFT),
    UserExecuteKernelReadExecute = (2ull << AP_SHIFT),
    UserReadExecuteKernelReadExecute = (3ull << AP_SHIFT),

    UserExecuteKernelReadWrite = (1ull << PNX_SHIFT) | (0ull << AP_SHIFT),
    UserExecuteKernelRead = (1ull << PNX_SHIFT) | (2ull << AP_SHIFT),
    UserReadExecuteKernelRead = (1ull << PNX_SHIFT) | (3ull << AP_SHIFT),

    UserNoneKernelReadWriteExecute = (1ull << UXN_SHIFT) | (0ull << AP_SHIFT),
    UserReadWriteKernelReadWrite = (1ull << UXN_SHIFT) | (1ull << AP_SHIFT),
    UserNoneKernelReadExecute = (1ull << UXN_SHIFT) | (2ull << AP_SHIFT),
    UserReadKernelReadExecute = (1ull << UXN_SHIFT) | (3ull << AP_SHIFT),

    UserNoneKernelReadWrite =
        (1ull << PNX_SHIFT) | (1ull << UXN_SHIFT) | (0ull << AP_SHIFT),
    UserNoneKernelRead =
        (1ull << PNX_SHIFT) | (1ull << UXN_SHIFT) | (2ull << AP_SHIFT),
    UserReadKernelRead =
        (1ull << PNX_SHIFT) | (1ull << UXN_SHIFT) | (3ull << AP_SHIFT),
};

constexpr u64 AP_FLAGS_MASK =
    (1ull << PNX_SHIFT) | (1ull << UXN_SHIFT) | (3ull << AP_SHIFT);

enum class AllocateVmMemoryError {
    AllocationFailed,
};

inline uptr AllocateVmMemory(usize size) {
    ASSERT_ALIGNMENT(size, APPLE_PAGE_SIZE, Hypervisor, "size")

    void* ptr;
    const auto res = posix_memalign(&ptr, APPLE_PAGE_SIZE, size);
    ASSERT_THROWING(res == 0, Hypervisor,
                    AllocateVmMemoryError::AllocationFailed,
                    "Failed to allocate memory: {:#x}", res);

    // Clear the memory
    memset(ptr, 0, size);

    return reinterpret_cast<uptr>(ptr);
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
