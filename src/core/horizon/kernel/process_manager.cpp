#include "core/horizon/kernel/process_manager.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::kernel {

ProcessManager::~ProcessManager() {
    for (auto process : processes) {
        if (process->IsRunning()) {
            LOG_FATAL(Kernel, "{} is still running", process->GetDebugName());
        } else {
            ASSERT(process->Release(), Kernel,
                   "Attempting to destroy {} which has active references",
                   process->GetDebugName());
        }
    }
}

Process* ProcessManager::CreateProcess(const std::string_view name) {
    std::lock_guard lock(mutex);
    Process* process = new Process(name);
    processes.push_back(process);
    return process;
}

void ProcessManager::DestroyProcess(Process* process) {
    std::lock_guard lock(mutex);
    processes.erase(std::remove(processes.begin(), processes.end(), process),
                    processes.end());
    ASSERT(process->Release(), Kernel,
           "Attempting to destroy {} which has active references",
           process->GetDebugName());
}

bool ProcessManager::HasRunningProcesses() {
    std::lock_guard lock(mutex);
    for (auto process : processes) {
        if (process->IsRunning())
            return true;
    }
    return false;
}

} // namespace hydra::horizon::kernel
