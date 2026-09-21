#include "core/horizon/kernel/shared_memory.hpp"

#include "core/hw/tegra_x1/cpu/cpu.hpp"

namespace hydra::horizon::kernel {

SharedMemory::SharedMemory(hw::tegra_x1::cpu::ICpu& cpu, u64 size,
                           std::string_view debug_name)
    : AutoObject(TYPE_ID, debug_name), memory{cpu.allocateMemory(size)} {
    // Clear memory
    memset(reinterpret_cast<void*>(getPtr()), 0, size);
}

SharedMemory::~SharedMemory() { delete memory; }

void SharedMemory::mapToRange(hw::tegra_x1::cpu::IMmu* mmu,
                              const ztd::Range<uptr> range,
                              MemoryPermission perm) {
    mmu->map(range.getBegin(), memory,
             {.type = MemoryType::Shared,
              .attr = MemoryAttribute::None,
              .perm = perm});
}

uptr SharedMemory::getPtr() const { return memory->getPtr(); }

} // namespace hydra::horizon::kernel
