#pragma once

#include "core/hw/tegra_x1/cpu/cpu_base.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"

namespace hydra::horizon {
class OS;
}

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class MMU;
class Thread;

class CPU : public CPUBase {
  public:
    CPU();
    ~CPU();

    ThreadBase* CreateThread(MemoryBase* tls_mem) override;

    // Getters
    MMUBase* GetMMU() const override;

  private:
    MMU* mmu;
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
