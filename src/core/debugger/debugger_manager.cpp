#include "core/debugger/debugger_manager.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::debugger {

constexpr hydra::horizon::kernel::Process* HYDRA_PROCESS = nullptr;

DebuggerManager::DebuggerManager() {
    // Hydra process
    hydra_debugger.RegisterThisThread("Main");

    // Callback
    LOGGER_INSTANCE.InstallCallback([this](const LogMessage& msg) {
        GetDebuggerForCurrentProcess().LogOnThisThread(msg);
    });
}

DebuggerManager::~DebuggerManager() {
    abort();
    // Callback
    LOGGER_INSTANCE.UninstallCallback();

    // Hydra process
    hydra_debugger.UnregisterThisThread();
}

void DebuggerManager::AttachDebugger(hydra::horizon::kernel::Process* process) {
    ASSERT(process != HYDRA_PROCESS, Debugger,
           "Debugger already attached to the Hydra process");

    // HACK
    // debuggers.emplace(process, Debugger());
    debuggers[process];
}

void DebuggerManager::DetachDebugger(hydra::horizon::kernel::Process* process) {
    ASSERT(process != HYDRA_PROCESS, Debugger,
           "Cannot detach debugger from the Hydra process");

    auto it = debuggers.find(process);
    ASSERT(it != debuggers.end(), Debugger, "Process \"{}\" not found",
           process->GetDebugName());
    debuggers.erase(it);
}

Debugger&
DebuggerManager::GetDebugger(hydra::horizon::kernel::Process* process) {
    if (process == HYDRA_PROCESS)
        return hydra_debugger;

    {
        std::lock_guard lock(mutex);
        auto it = debuggers.find(process);
        ASSERT_DEBUG(it != debuggers.end(), Debugger,
                     "Process \"{}\" not found", process->GetDebugName());
        return it->second;
    }
}

Debugger& DebuggerManager::GetDebuggerForCurrentProcess() {
    // Get the corresponding process
    auto process = HYDRA_PROCESS;
    if (horizon::kernel::tls_current_thread)
        process = horizon::kernel::tls_current_thread->GetProcess();

    return GetDebugger(process);
}

} // namespace hydra::debugger
