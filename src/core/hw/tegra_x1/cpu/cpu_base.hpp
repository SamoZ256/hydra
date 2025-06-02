#pragma once

namespace hydra::hw::tegra_x1::cpu {

class MemoryBase;
class MMUBase;
class ThreadBase;

class CPUBase {
  public:
    static CPUBase& GetInstance();

    CPUBase();
    virtual ~CPUBase();

    virtual ThreadBase* CreateThread(MemoryBase* tls_mem) = 0;

    virtual MMUBase* GetMMU() const = 0;
};

} // namespace hydra::hw::tegra_x1::cpu
