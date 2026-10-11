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
    u32 breakpoint_slots;
    u32 watchpoint_slots;
};

class ICpu {
  public:
    virtual ~ICpu() = default;

    virtual IMmu* createMmu(System& system) = 0;
    virtual IThread* createThread(WallClock& wall_clock, IMmu* mmu,
                                  const ThreadCallbacks& callbacks,
                                  IMemory* tls_mem, vaddr_t tls_mem_base) = 0;
    virtual IMemory* allocateMemory(u64 size) = 0;

  protected:
    CpuFeatures features;

  public:
    CONST_REF_GETTER(features, getFeatures);
};

} // namespace hydra::hw::tegra_x1::cpu
