#include "core/horizon/kernel/guest_thread.hpp"

#include "core/debugger/debugger_manager.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"
#include "core/system.hpp"

namespace hydra::horizon::kernel {

GuestThread::GuestThread(System& system_, Process* process,
                         vaddr_t stack_top_addr_, i32 priority,
                         std::string_view debug_name)
    : IThread(process, priority, debug_name), system{system_},
      stack_top_addr{stack_top_addr_} {
    tls_mem = process->createTlsMemory(tls_addr);
}

GuestThread::~GuestThread() { delete tls_mem; }

uptr GuestThread::getTlsPtr() const { return tls_mem->getPtr(); }

void GuestThread::run() {
    // Create
    ASSERT(entry_point != invalid<vaddr_t>(), Kernel, "Invalid entry point");
    thread = system.getCpu().createThread(
        system.getWallClock(), process->getMmu(),
        {.svc_handler =
             [this](hw::tegra_x1::cpu::IThread* hw_thread, u64 id) {
                 system.getOs().getKernel().supervisorCall(process, this,
                                                           hw_thread, id);
             },
         .stop_requested =
             [this] {
                 processMessages();
                 return getState() == ThreadState::Stopping;
             },
         .supervisor_pause =
             [this] {
                 supervisorPause();
                 DEBUGGER_MANAGER_INSTANCE.getDebugger(process)
                     .notifySupervisorPaused(this, debugger::Signal::SigTrap);
             },
         .breakpoint_hit =
             [this] {
                 DEBUGGER_MANAGER_INSTANCE.getDebugger(process).breakpointHit(
                     this);
             }},
        tls_mem, tls_addr);

    // Initialize state
    auto& state = thread->getState();
    state.pc = entry_point;
    state.sp = stack_top_addr;
    state.lr = return_address;
    for (u32 i = 0; i < SIZEOF_ARRAY(args); i++)
        state.r[i] = args[i];

    // Run
    GET_CURRENT_PROCESS_DEBUGGER().registerGuestThreadForThisThread(this);
    thread->run();
    GET_CURRENT_PROCESS_DEBUGGER().unregisterGuestThreadForThisThread();

    // Cleanup
    delete thread;
    thread = nullptr;
}

} // namespace hydra::horizon::kernel
