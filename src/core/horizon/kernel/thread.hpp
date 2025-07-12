#pragma once

#include "core/horizon/kernel/const.hpp"
#include "core/horizon/kernel/synchronization_object.hpp"
#include "core/hw/tegra_x1/cpu/memory.hpp"

namespace hydra::horizon::kernel {

class Process;

class Thread : public SynchronizationObject {
  public:
    Thread(Process* process_, vaddr_t stack_top_addr_, i32 priority_,
           const std::string_view debug_name = "Thread");
    ~Thread() override;

    void Start();
    void RequestStop();

    // Setters
    void SetEntryPoint(vaddr_t entry_point_) { entry_point = entry_point_; }
    void SetArg(u32 index, u64 value) {
        ASSERT(index < sizeof_array(args), Kernel, "Invalid argument index {}",
               index);
        args[index] = value;
    }

  private:
    Process* process;

    hw::tegra_x1::cpu::IMemory* tls_mem;
    vaddr_t tls_addr;
    vaddr_t stack_top_addr;
    i32 priority;

    vaddr_t entry_point{0};
    u64 args[2] = {0};

    std::thread* thread{nullptr};
    std::atomic<bool> stop_requested{false};
};

} // namespace hydra::horizon::kernel
