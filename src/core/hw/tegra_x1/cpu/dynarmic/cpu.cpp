#include "core/hw/tegra_x1/cpu/dynarmic/cpu.hpp"

#include "core/hw/tegra_x1/cpu/dynarmic/memory.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/thread.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

Cpu::Cpu() {
    // Features
    features = {.breakpoint_slots = 0, .watchpoint_slots = 0};
}

IMmu* Cpu::createMmu(System& system) { return new Mmu(system); }

IThread* Cpu::createThread(WallClock& wall_clock, IMmu* mmu,
                           const ThreadCallbacks& callbacks, IMemory* tls_mem,
                           vaddr_t tls_mem_base) {
    return new Thread(wall_clock, mmu, callbacks, tls_mem, tls_mem_base);
}

IMemory* Cpu::allocateMemory(u64 size) {
    size = align(size, GUEST_PAGE_SIZE);
    auto memory = new Memory(size);

    return memory;
}

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
