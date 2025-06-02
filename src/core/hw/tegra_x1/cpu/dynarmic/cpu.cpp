#include "core/hw/tegra_x1/cpu/dynarmic/cpu.hpp"

#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/thread.hpp"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

CPU::CPU() {
    // TODO

    // MMU
    mmu = new MMU();
}

CPU::~CPU() {
    // TODO
}

ThreadBase* CPU::CreateThread(MemoryBase* tls_mem) {
    Thread* thread = new Thread(mmu, tls_mem);
    // TODO

    return thread;
}

MMUBase* CPU::GetMMU() const { return mmu; }

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
