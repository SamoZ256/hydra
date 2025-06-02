#pragma once

#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/thread.hpp"
#include "core/hw/tegra_x1/cpu/memory_base.hpp"

namespace hydra::horizon::kernel {

class Process : public Handle {
  public:
    Process();
    ~Process() override;

    void Run();

    // Getter
    HandleWithId<Thread>& GetMainThread() { return main_thread; }

  private:
    HandleWithId<Thread> main_thread;
    hw::tegra_x1::cpu::MemoryBase* stack_mem;
};

} // namespace hydra::horizon::kernel
