#include "core/horizon/kernel/shared_memory.hpp"

#include "core/hw/tegra_x1/cpu/mmu_base.hpp"

namespace hydra::horizon::kernel {

SharedMemory::SharedMemory(usize size) {
    memory = hw::tegra_x1::cpu::MMUBase::GetInstance().AllocateMemory(size);

    // Clear
    memset(reinterpret_cast<void*>(GetPtr()), 0, size);
}

SharedMemory::~SharedMemory() {
    hw::tegra_x1::cpu::MMUBase::GetInstance().FreeMemory(memory);
}

void SharedMemory::MapToRange(
    const range<uptr> range,
    MemoryPermission perm) { // TODO: why ::hydra::range?
    hw::tegra_x1::cpu::MMUBase::GetInstance().Map(
        range.begin, memory, {MemoryType::Shared, MemoryAttribute::None, perm});
}

uptr SharedMemory::GetPtr() const {
    return hw::tegra_x1::cpu::MMUBase::GetInstance().GetMemoryPtr(memory);
}

} // namespace hydra::horizon::kernel
