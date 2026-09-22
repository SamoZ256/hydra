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

class Cpu : public ICpu {
  public:
    Cpu();

    IMmu* createMmu(System& system) override;
    IThread* createThread(WallClock& wall_clock, IMmu* mmu,
                          const ThreadCallbacks& callbacks, IMemory* tls_mem,
                          vaddr_t tls_mem_base) override;
    IMemory* allocateMemory(u64 size) override;
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
