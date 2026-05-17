#include "core/horizon/kernel/shared_memory.hpp"

#include "core/hw/tegra_x1/cpu/cpu.hpp"

namespace hydra::horizon::kernel {

SharedMemory::SharedMemory(hw::tegra_x1::cpu::ICpu& cpu, usize size,
                           const std::string_view debug_name)
    : AutoObject(debug_name) {
    memory = cpu.AllocateMemory(size);

    // Clear
    memset(reinterpret_cast<void*>(GetPtr()), 0, size);
}

SharedMemory::~SharedMemory() { delete memory; }

void SharedMemory::MapToRange(hw::tegra_x1::cpu::IMmu* mmu,
                              const Range<uptr> range, MemoryPermission perm) {
    mmu->Map(range.GetBegin(), memory,
             {MemoryType::Shared, MemoryAttribute::None, perm});
}

uptr SharedMemory::GetPtr() const { return memory->GetPtr(); }

} // namespace hydra::horizon::kernel
