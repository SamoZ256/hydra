#include "core/horizon/kernel/guest_thread.hpp"

#include "core/debugger/debugger.hpp"
#include "core/horizon/kernel/kernel.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::horizon::kernel {

GuestThread::GuestThread(Process* process, vaddr_t stack_top_addr_,
                         i32 priority, const std::string_view debug_name)
    : IThread(process, priority, debug_name), stack_top_addr{stack_top_addr_} {
    tls_mem = process->CreateTlsMemory(tls_addr);
}

GuestThread::~GuestThread() { delete tls_mem; }

uptr GuestThread::GetTlsPtr() const { return tls_mem->GetPtr(); }

void GuestThread::Run() {
    ASSERT(entry_point != 0x0, Kernel, "Invalid entry point");

    auto thread = CPU_INSTANCE.CreateThread(
        process->GetMmu(),
        [this](hw::tegra_x1::cpu::IThread* thread, u64 id) {
            KERNEL_INSTANCE.SupervisorCall(process, this, thread, id);
        },
        [this]() {
            ProcessMessages();
            return GetState() == ThreadState::Stopping;
        },
        tls_mem, tls_addr, stack_top_addr);

    thread->SetPC(entry_point);
    for (u32 i = 0; i < sizeof_array(args); i++)
        thread->SetRegX(i, args[i]);

    DEBUGGER_INSTANCE.RegisterThisThread(GetDebugName(), thread);
    thread->Run();
    DEBUGGER_INSTANCE.UnregisterThisThread();

    delete thread;
}

} // namespace hydra::horizon::kernel
