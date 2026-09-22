#include "core/debugger/debugger_manager.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::debugger {

constexpr hydra::horizon::kernel::Process* HYDRA_PROCESS = nullptr;

// TODO: lazy initialize or smh
static DebuggerManager g_instance;

DebuggerManager& DebuggerManager::getInstance() { return g_instance; }

DebuggerManager::DebuggerManager() : hydra_debugger("Hydra", HYDRA_PROCESS) {
    // Hydra process
    hydra_debugger.registerThisThread("Main");

    // Callback
    LOGGER_INSTANCE.installCallback([this](const LogMessage& msg) {
        getDebuggerForCurrentProcess().logOnThisThread(msg);
    });
}

DebuggerManager::~DebuggerManager() {
    // Callback
    LOGGER_INSTANCE.uninstallCallback();

    // Hydra process
    hydra_debugger.unregisterThisThread();
}

void DebuggerManager::attachDebugger(hydra::horizon::kernel::Process* process,
                                     const std::string_view name) {
    ASSERT(process != HYDRA_PROCESS, Debugger,
           "Debugger already attached to the Hydra process");

    debuggers.try_emplace(process, name, process);
}

void DebuggerManager::detachDebugger(hydra::horizon::kernel::Process* process) {
    ASSERT(process != HYDRA_PROCESS, Debugger,
           "Cannot detach debugger from the Hydra process");

    auto it = debuggers.find(process);
    ASSERT(it != debuggers.end(), Debugger, "Process \"{}\" not found",
           process->getDebugName());
    debuggers.erase(it);
}

Debugger&
DebuggerManager::getDebugger(hydra::horizon::kernel::Process* process) {
    if (process == HYDRA_PROCESS)
        return hydra_debugger;

    {
        std::scoped_lock lock(mutex);
        auto it = debuggers.find(process);
        ASSERT_DEBUG(it != debuggers.end(), Debugger,
                     "Process \"{}\" not found", process->getDebugName());
        return it->second;
    }
}

Debugger& DebuggerManager::getDebuggerForCurrentProcess() {
    // Get the corresponding process
    auto process = HYDRA_PROCESS;
    if (horizon::kernel::tls_current_thread != nullptr)
        process = horizon::kernel::tls_current_thread->getProcess();

    return getDebugger(process);
}

} // namespace hydra::debugger
