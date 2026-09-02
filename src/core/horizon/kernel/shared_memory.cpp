#include "core/horizon/kernel/shared_memory.hpp"

#include "core/hw/tegra_x1/cpu/cpu.hpp"

namespace hydra::horizon::kernel {

SharedMemory::SharedMemory(hw::tegra_x1::cpu::ICpu& cpu, u64 size,
                           std::string_view debug_name)
    : AutoObject(TYPE_ID, debug_name), memory{cpu.AllocateMemory(size)} {
    // Clear memory
    memset(reinterpret_cast<void*>(GetPtr()), 0, size);
}

SharedMemory::~SharedMemory() { delete memory; }

void SharedMemory::MapToRange(hw::tegra_x1::cpu::IMmu* mmu,
                              const ztd::Range<uptr> range, MemoryPermission perm) {
    mmu->Map(range.getBegin(), memory,
             {.type = MemoryType::Shared,
              .attr = MemoryAttribute::None,
              .perm = perm});
}

uptr SharedMemory::GetPtr() const { return memory->GetPtr(); }

} // namespace hydra::horizon::kernel
