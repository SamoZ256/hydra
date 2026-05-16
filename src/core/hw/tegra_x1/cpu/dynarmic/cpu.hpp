#pragma once

#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/dynarmic/const.hpp"

namespace hydra::horizon {
class OS;
}

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::cpu::dynarmic {

class Mmu;
class Thread;

class Cpu : public ICpu {
  public:
    Cpu();

    IMmu* CreateMmu(System& system) override;
    IThread* CreateThread(WallClock& wall_clock, IMmu* mmu,
                          const ThreadCallbacks& callbacks, IMemory* tls_mem,
                          vaddr_t tls_mem_base) override;
    IMemory* AllocateMemory(usize size) override;
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
