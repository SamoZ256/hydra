#include "core/horizon/kernel/thread.hpp"

#include "core/debugger/debugger.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::horizon::kernel {

Thread::Thread(Process* process_, vaddr_t stack_top_addr_, i32 priority_,
               const std::string_view debug_name)
    : SynchronizationObject(false, debug_name), process{process_},
      stack_top_addr{stack_top_addr_}, priority{priority_} {
    tls_mem = process->CreateTlsMemory(tls_addr);
}

Thread::~Thread() {
    if (thread) {
        // Request stop
        stop_requested = true;
        thread->join();
        delete thread;
    }
    delete tls_mem;
}

void Thread::Start() {
    ASSERT(entry_point != 0x0, Kernel, "Invalid entry point");

    thread = new std::thread([&]() {
        auto thread = CPU_INSTANCE.CreateThread(
            process->GetMmu(),
            [this](hw::tegra_x1::cpu::IThread* thread, u64 id) {
                KERNEL_INSTANCE.SupervisorCall(process, this, thread, id);
            },
            [this]() { return bool(stop_requested); }, tls_mem, tls_addr,
            stack_top_addr);

        process->RegisterThread(this);
        DEBUGGER_INSTANCE.RegisterThisThread("Guest",
                                             thread); // TODO: handle ID?

        thread->SetPC(entry_point);
        for (u32 i = 0; i < sizeof_array(args); i++)
            thread->SetRegX(i, args[i]);

        thread->Run();

        process->UnregisterThread(this);
        DEBUGGER_INSTANCE.UnregisterThisThread();

        delete thread;
    });
}

void Thread::RequestStop() { stop_requested = true; }

} // namespace hydra::horizon::kernel
