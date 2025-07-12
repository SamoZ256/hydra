#include "core/horizon/kernel/process_manager.hpp"

#include "core/horizon/kernel/process.hpp"

namespace hydra::horizon::kernel {

ProcessManager::~ProcessManager() {
    ASSERT(processes.empty(), Kernel, "Processes are still running");
}

Process* ProcessManager::CreateProcess(const std::string_view name) {
    std::lock_guard<std::mutex> lock(mutex);
    Process* process = new Process(name);
    processes.push_back(process);
    return process;
}

void ProcessManager::DestroyProcess(Process* process) {
    std::lock_guard<std::mutex> lock(mutex);
    processes.erase(std::remove(processes.begin(), processes.end(), process),
                    processes.end());
    delete process;
}

bool ProcessManager::HasRunningProcesses() {
    std::lock_guard<std::mutex> lock(mutex);
    for (const auto& process : processes) {
        if (process->IsRunning())
            return true;
    }
    return false;
}

} // namespace hydra::horizon::kernel
