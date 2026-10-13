#pragma once

#include <set>

#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::horizon::kernel {
class GuestThread;
class Process;
} // namespace hydra::horizon::kernel

namespace hydra::hw::tegra_x1::cpu {
class IMmu;
} // namespace hydra::hw::tegra_x1::cpu

namespace hydra::debugger {

using thread_id_t = u32;

constexpr i64 DEFAULT_STOP_TIMEOUT_NS = 100ll * 1000ll * 1000ll;

class Target;

class StoppedThread {
    friend class Target;

  public:
    ZTD_MAKE_NON_COPYABLE(StoppedThread);
    ZTD_MAKE_DEFAULT_MOVABLE(StoppedThread);
    ~StoppedThread() noexcept = default;

    hw::tegra_x1::cpu::ThreadState& getState() const {
        return cpu_thread->getState();
    }

  private:
    StoppedThread(std::unique_lock<std::mutex>&& lock_,
                  hw::tegra_x1::cpu::IThread* cpu_thread_)
        : lock{std::move(lock_)}, cpu_thread{cpu_thread_} {}

    std::unique_lock<std::mutex> lock;
    hw::tegra_x1::cpu::IThread* cpu_thread;
};

struct Breakpoint {
    u32 replaced_instruction{0};
};

struct StopInfo {
    hw::tegra_x1::cpu::HaltReason reason;
    u64 sequence;
    std::optional<hw::tegra_x1::cpu::Watchpoint> watchpoint{std::nullopt};
};

struct ThreadSnapshot {
    thread_id_t id;
    std::optional<StopInfo> stop;
    bool resumed;
};

enum class ResumeKind {
    Continue,
    Step,
};

struct ResumeAction {
    thread_id_t id;
    ResumeKind kind;
};

using halt_state_observer_fn_t =
    std::function<void(thread_id_t, const std::optional<StopInfo>&)>;

class Target {
  public:
    explicit Target(horizon::kernel::Process& process_);
    ~Target() noexcept = default;

    ZTD_MAKE_NON_COPYABLE(Target);
    ZTD_MAKE_NON_MOVABLE(Target);

    // Threads
    thread_id_t registerThread(horizon::kernel::GuestThread* guest_thread);
    void unregisterThread(thread_id_t id);

    std::vector<thread_id_t> getThreadIds() const;
    bool hasThread(thread_id_t id) const;
    std::optional<StopInfo> getStopInfo(thread_id_t id) const;
    std::vector<ThreadSnapshot> getThreadSnapshots() const;

    // Execution
    void pauseAll();
    void resumeAll();
    bool resume(std::span<const ResumeAction> actions);
    void stop();
    bool isStopping() const;
    bool isAllStopped() const;

    void notifyHalted(horizon::kernel::GuestThread* guest_thread,
                      const hw::tegra_x1::cpu::HaltInfo& halt);

    // Registers
    std::optional<StoppedThread>
    tryAcquireStoppedThread(thread_id_t id,
                            i64 timeout_ns = DEFAULT_STOP_TIMEOUT_NS);

    // Memory
    bool tryReadMemory(vaddr_t addr, std::span<u8> out_data) const;
    bool tryWriteMemory(vaddr_t addr, std::span<const u8> data);

    // Breakpoints
    bool insertBreakpoint(vaddr_t addr);
    bool removeBreakpoint(vaddr_t addr);
    void removeAllBreakpoints();
    bool hasBreakpoint(vaddr_t addr) const;
    std::vector<vaddr_t> getBreakpoints() const;

    // Watchpoints
    bool insertWatchpoint(const hw::tegra_x1::cpu::Watchpoint& watchpoint);
    bool removeWatchpoint(const hw::tegra_x1::cpu::Watchpoint& watchpoint);
    void removeAllWatchpoints();

    bool isProcessRunning() const;
    u64 getTitleId() const;

  private:
    struct ThreadEntry {
        horizon::kernel::GuestThread* guest_thread;
        hw::tegra_x1::cpu::IThread* cpu_thread;
        std::optional<StopInfo> stop{std::nullopt};
        bool stepping{false};
        bool resumed{true};
    };

    horizon::kernel::Process& process;
    const u32 breakpoint_slots;
    const u32 watchpoint_slots;
    const bool native_breakpoints;

    mutable std::mutex run_control_mutex;
    mutable std::mutex mutex;
    std::map<thread_id_t, ThreadEntry> threads;
    thread_id_t next_thread_id{1};
    std::map<vaddr_t, Breakpoint> breakpoints;
    std::set<hw::tegra_x1::cpu::Watchpoint> watchpoints;
    bool stopping{false};
    bool partial_resume{false};
    u64 stop_sequence{0};
    halt_state_observer_fn_t halt_state_observer;

    // Helpers
    ThreadEntry* findEntry(thread_id_t id);
    const ThreadEntry* findEntry(thread_id_t id) const;
    bool isStaleTrap(hw::tegra_x1::cpu::HaltReason reason, vaddr_t pc) const;
    bool isStateAccessible(const ThreadEntry& entry) const;
    void kickRunningThreads();
    bool removeBreakpointLocked(std::map<vaddr_t, Breakpoint>::iterator it);
    std::optional<hw::tegra_x1::cpu::Watchpoint>
    findWatchpoint(vaddr_t access_addr) const;

    bool tryReadMemoryRaw(vaddr_t addr, std::span<u8> out_data) const;
    bool tryWriteMemoryRaw(vaddr_t addr, std::span<const u8> data);
    void readBreakpointShadow(vaddr_t addr, std::span<u8> data) const;
    void writeBreakpointShadow(vaddr_t addr, std::span<const u8> data);
    void notifyMemoryChanged(ztd::Range<vaddr_t> mem_range);

    hw::tegra_x1::cpu::IMmu* getMmu() const;

  public:
    GETTER(breakpoint_slots, getBreakpointSlotCount);
    GETTER(watchpoint_slots, getWatchpointSlotCount);
    GETTER(native_breakpoints, hasNativeBreakpoints);
    CONST_REF_SETTER(halt_state_observer, setHaltStateObserver);
};

} // namespace hydra::debugger
