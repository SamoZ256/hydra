#pragma once

#include "core/horizon/kernel/thread.hpp"

namespace hydra {
class System;
}

namespace hydra::hw::tegra_x1::cpu {
class IMemory;
class IThread;
} // namespace hydra::hw::tegra_x1::cpu

namespace hydra::horizon::kernel {

class GuestThread : public IThread {
  public:
    GuestThread(System& system_, Process* process, vaddr_t stack_top_addr_,
                i32 priority, std::string_view debug_name = "Guest thread");
    ~GuestThread() override;

    void setEntryPoint(vaddr_t entry_point_) { entry_point = entry_point_; }
    void setReturnAddress(vaddr_t return_address_) {
        return_address = return_address_;
    }
    void setArg(u32 index, u64 value) {
        ASSERT(index < SIZEOF_ARRAY(args), Kernel, "Invalid argument index {}",
               index);
        args[index] = value;
    }

    uptr getTlsPtr() const override;

  protected:
    vaddr_t entry_point{invalid<vaddr_t>()};
    vaddr_t return_address{invalid<vaddr_t>()};
    u64 args[2] = {0};

    void run() override;

  private:
    System& system;

    hw::tegra_x1::cpu::IMemory* tls_mem;
    vaddr_t tls_addr;
    vaddr_t stack_top_addr;

    hw::tegra_x1::cpu::IThread* thread{nullptr};

  public:
    GETTER(thread, getThread);
};

} // namespace hydra::horizon::kernel
