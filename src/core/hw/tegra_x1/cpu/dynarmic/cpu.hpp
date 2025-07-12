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
    IMmu* CreateMmu() override;
    IThread* CreateThread(IMmu* mmu, const svc_handler_fn_t& svc_handler,
                          const stop_requested_fn_t& stop_requested,
                          IMemory* tls_mem, vaddr_t tls_mem_base,
                          vaddr_t stack_mem_end) override;
    IMemory* AllocateMemory(usize size) override;
};

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
