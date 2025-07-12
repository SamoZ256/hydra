#pragma once

#include "core/hw/tegra_x1/cpu/thread.hpp"

#define CPU_INSTANCE hw::tegra_x1::cpu::ICpu::GetInstance()

namespace hydra::hw::tegra_x1::cpu {

class IMemory;
class IMmu;
class IThread;

class ICpu {
  public:
    static ICpu& GetInstance();

    ICpu();
    virtual ~ICpu();

    virtual IMmu* CreateMmu() = 0;
    virtual IThread* CreateThread(IMmu* mmu,
                                  const svc_handler_fn_t& svc_handler,
                                  const stop_requested_fn_t& stop_requested,
                                  IMemory* tls_mem, vaddr_t tls_mem_base,
                                  vaddr_t stack_mem_end) = 0;
    virtual IMemory* AllocateMemory(usize size) = 0;
};

} // namespace hydra::hw::tegra_x1::cpu
