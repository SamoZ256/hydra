#pragma once

#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra {
class System;
}

namespace hydra::hw::tegra_x1::cpu {

class IMemory;
class IMmu;
class IThread;

struct CpuFeatures {
    bool supports_native_breakpoints;
    bool supports_synchronous_single_step;
};

class ICpu {
  public:
    virtual ~ICpu() = default;

    virtual IMmu* CreateMmu(System& system) = 0;
    virtual IThread* CreateThread(WallClock& wall_clock, IMmu* mmu,
                                  const ThreadCallbacks& callbacks,
                                  IMemory* tls_mem, vaddr_t tls_mem_base) = 0;
    virtual IMemory* AllocateMemory(u64 size) = 0;

  protected:
    CpuFeatures features;

  public:
    CONST_REF_GETTER(features, GetFeatures);
};

} // namespace hydra::hw::tegra_x1::cpu
