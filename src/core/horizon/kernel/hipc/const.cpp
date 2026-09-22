#include "core/horizon/kernel/hipc/const.hpp"

#include "core/hw/tegra_x1/cpu/mmu.hpp"

namespace hydra::horizon::kernel::hipc {

u8* getBufferPtr(const hw::tegra_x1::cpu::IMmu* mmu,
                 const BufferDescriptor& descriptor, u64& size) {
    uptr addr = descriptor.address_low |
                static_cast<u64>(descriptor.address_mid) << 32 |
                static_cast<u64>(descriptor.address_high) << 36;
    if (addr == 0x0)
        return nullptr;

    size = descriptor.size_low | static_cast<u64>(descriptor.size_high) << 32;
    if (size == 0x0)
        return nullptr;

    return reinterpret_cast<u8*>(mmu->unmapAddr(addr));
}

u8* getStaticPtr(const hw::tegra_x1::cpu::IMmu* mmu,
                 const StaticDescriptor& descriptor, u64& size) {
    uptr addr = descriptor.address_low |
                static_cast<u64>(descriptor.address_mid) << 32 |
                static_cast<u64>(descriptor.address_high) << 36;
    if (addr == 0x0)
        return nullptr;

    size = descriptor.size;
    if (size == 0x0)
        return nullptr;

    return reinterpret_cast<u8*>(mmu->unmapAddr(addr));
}

u8* getListEntryPtr(const hw::tegra_x1::cpu::IMmu* mmu,
                    const RecvListEntry& descriptor, u64& size) {
    uptr addr = descriptor.address_low |
                static_cast<u64>(descriptor.address_high) << 32;
    if (addr == 0x0)
        return nullptr;

    size = descriptor.size;
    if (size == 0x0)
        return nullptr;

    return reinterpret_cast<u8*>(mmu->unmapAddr(addr));
}

} // namespace hydra::horizon::kernel::hipc
