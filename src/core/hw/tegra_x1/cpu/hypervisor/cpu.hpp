#pragma once

#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/memory.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/page_table.hpp"

namespace hydra::horizon {
class OS;
}

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
}

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class Mmu;
class Thread;

class VirtualMachine {
  public:
    VirtualMachine();
    ~VirtualMachine();
};

class Cpu : public ICpu {
  public:
    Cpu();
    ~Cpu();

    IMmu* CreateMmu() override;
    IThread* CreateThread(IMmu* mmu, const svc_handler_fn_t& svc_handler,
                          const stop_requested_fn_t& stop_requested,
                          IMemory* tls_mem, vaddr_t tls_mem_base,
                          vaddr_t stack_mem_end) override;
    IMemory* AllocateMemory(usize size) override;

  private:
    VirtualMachine vm;
    Memory kernel_mem;
    PageTable kernel_page_table;

  public:
    CONST_REF_GETTER(kernel_page_table, GetKernelPageTable);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
