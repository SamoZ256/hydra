#pragma once

#include "core/horizon/kernel/const.hpp"
#include "core/hw/tegra_x1/cpu/memory_base.hpp"

namespace hydra::horizon::kernel {

class Thread : public Handle {
  public:
    Thread(vaddr_t stack_top_addr_, i32 priority_);
    ~Thread() override;

    void Run();

    // Setters
    void SetEntryPoint(vaddr_t entry_point_) { entry_point = entry_point_; }
    void SetArg(u32 index, u64 value) {
        ASSERT(index < sizeof_array(args), Kernel, "Invalid argument index {}",
               index);
        args[index] = value;
    }

  private:
    hw::tegra_x1::cpu::MemoryBase* tls_mem;
    vaddr_t tls_addr;
    vaddr_t stack_top_addr;
    i32 priority;
    vaddr_t entry_point{0};
    u64 args[2] = {0};

    std::thread* t = nullptr;
};

} // namespace hydra::horizon::kernel
