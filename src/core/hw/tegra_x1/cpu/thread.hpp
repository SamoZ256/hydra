#pragma once

#include "core/hw/tegra_x1/cpu/const.hpp"

namespace hydra::hw {
class WallClock;
}

namespace hydra::hw::tegra_x1::cpu {

class IMemory;
class IMmu;
class IThread;

struct ThreadCallbacks {
    std::function<void(IThread*, u64)> svc_handler;
    std::function<bool()> stop_requested;
    std::function<void()> supervisor_pause;
    std::function<void()> breakpoint_hit;
};

using stack_frame_callback_fn_t = std::function<void(vaddr_t)>;

struct ThreadState {
    u64 r[29];
    u64 fp;
    u64 lr;
    u64 sp;
    u64 pc;
    u32 pstate;
    u128 v[32];
    u32 fpcr;
    u32 fpsr;
};

class IThread {
  public:
    IThread(WallClock& wall_clock_, IMmu* mmu_, ThreadCallbacks callbacks_,
            IMemory* tls_mem_)
        : wall_clock{wall_clock_}, mmu{mmu_}, callbacks{std::move(callbacks_)},
          tls_mem{tls_mem_} {}
    virtual ~IThread() noexcept = default;

    virtual void Run() = 0;

    virtual void
    NotifyMemoryChanged([[maybe_unused]] Range<vaddr_t> mem_range) {}

    // Debug
    void GetStackTrace(const stack_frame_callback_fn_t& callback);

    virtual void InsertBreakpoint(vaddr_t addr) = 0;
    virtual void RemoveBreakpoint(vaddr_t addr) = 0;
    virtual void SingleStep() = 0;

    // Getters
    IMemory* GetTlsMemory() const { return tls_mem; }

  protected:
    WallClock& wall_clock;
    IMmu* mmu;
    ThreadCallbacks callbacks;
    IMemory* tls_mem;

    ThreadState state{};

  public:
    REF_GETTER(state, GetState);
};

} // namespace hydra::hw::tegra_x1::cpu
