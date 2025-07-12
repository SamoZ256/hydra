#include "core/horizon/kernel/shared_memory.hpp"

#include "core/hw/tegra_x1/cpu/cpu.hpp"

namespace hydra::horizon::kernel {

SharedMemory::SharedMemory(usize size, const std::string_view debug_name)
    : AutoObject(debug_name) {
    memory = CPU_INSTANCE.AllocateMemory(size);

    // Clear
    memset(reinterpret_cast<void*>(GetPtr()), 0, size);
}

SharedMemory::~SharedMemory() { delete memory; }

void SharedMemory::MapToRange(hw::tegra_x1::cpu::IMmu* mmu,
                              const range<uptr> range, MemoryPermission perm) {
    mmu->Map(range.begin, memory,
             {MemoryType::Shared, MemoryAttribute::None, perm});
}

uptr SharedMemory::GetPtr() const { return memory->GetPtr(); }

} // namespace hydra::horizon::kernel
