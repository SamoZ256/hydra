#include "core/hw/tegra_x1/cpu/dynarmic/cpu.hpp"

#include "core/hw/tegra_x1/cpu/dynarmic/memory.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/thread.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

Cpu::Cpu() {
    // Features
    features = {.supports_native_breakpoints = false,
                .supports_synchronous_single_step = true};
}

IMmu* Cpu::CreateMmu(System& system) { return new Mmu(system); }

IThread* Cpu::CreateThread(WallClock& wall_clock, IMmu* mmu,
                           const ThreadCallbacks& callbacks, IMemory* tls_mem,
                           vaddr_t tls_mem_base) {
    return new Thread(wall_clock, mmu, callbacks, tls_mem, tls_mem_base);
}

IMemory* Cpu::AllocateMemory(u64 size) {
    size = align(size, GUEST_PAGE_SIZE);
    auto memory = new Memory(size);

    return memory;
}

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
