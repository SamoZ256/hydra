#pragma once

#include "hw/tegra_x1/cpu/cpu_base.hpp"
#include "hw/tegra_x1/cpu/hypervisor/const.hpp"

namespace Hydra::Horizon {
class OS;
}

namespace Hydra::HW::TegraX1::CPU::Hypervisor {

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

} // namespace Hydra::HW::TegraX1::CPU::Hypervisor
