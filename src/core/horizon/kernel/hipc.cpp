#include "core/horizon/kernel/hipc.hpp"

#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::horizon::kernel::hipc {

u8* get_buffer_ptr(const hw::tegra_x1::cpu::MMUBase* mmu,
                   const BufferDescriptor& descriptor, usize& size) {
    uptr addr = descriptor.address_low | (u64)descriptor.address_mid << 32 |
                (u64)descriptor.address_high << 36;
    if (addr == 0x0)
        return nullptr;

    size = descriptor.size_low | (usize)descriptor.size_high << 32;
    if (size == 0x0)
        return nullptr;

    return reinterpret_cast<u8*>(mmu->UnmapAddr(addr));
}

u8* get_static_ptr(const hw::tegra_x1::cpu::MMUBase* mmu,
                   const StaticDescriptor& descriptor, usize& size) {
    uptr addr = descriptor.address_low | (u64)descriptor.address_mid << 32 |
                (u64)descriptor.address_high << 36;
    if (addr == 0x0)
        return nullptr;

    size = descriptor.size;
    if (size == 0x0)
        return nullptr;

    return reinterpret_cast<u8*>(mmu->UnmapAddr(addr));
}

u8* get_list_entry_ptr(const hw::tegra_x1::cpu::MMUBase* mmu,
                       const RecvListEntry& descriptor, usize& size) {
    uptr addr = descriptor.address_low | (u64)descriptor.address_high << 32;
    if (addr == 0x0)
        return nullptr;

    size = descriptor.size;
    if (size == 0x0)
        return nullptr;

    return reinterpret_cast<u8*>(mmu->UnmapAddr(addr));
}

} // namespace hydra::horizon::kernel::hipc
