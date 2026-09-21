#include "core/horizon/kernel/process_manager.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::kernel {

ProcessManager::~ProcessManager() {
    for (auto process : processes) {
        if (process->isRunning()) {
            LOG_FATAL(Kernel, "{} is still running", process->getDebugName());
        } else {
            ASSERT(process->release(), Kernel,
                   "Attempting to destroy {} which has active references",
                   process->getDebugName());
        }
    }
}

Process* ProcessManager::createProcess(const std::string_view name) {
    std::scoped_lock lock(mutex);
    auto process = new Process(system, name);
    processes.push_back(process);
    return process;
}

void ProcessManager::destroyProcess(Process* process) {
    std::scoped_lock lock(mutex);
    std::erase(processes, process);
    ASSERT(process->release(), Kernel,
           "Attempting to destroy {} which has active references",
           process->getDebugName());
}

bool ProcessManager::hasRunningProcesses() {
    std::scoped_lock lock(mutex);
    for (auto process : processes) {
        if (process->isRunning())
            return true;
    }
    return false;
}

} // namespace hydra::horizon::kernel
