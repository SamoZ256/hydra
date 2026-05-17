#pragma once

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {

class Process;

class ProcessManager {
  public:
    ProcessManager(System& system_) : system{system_} {}
    ~ProcessManager();

    Process* CreateProcess(const std::string_view name);
    void DestroyProcess(Process* process);

    bool HasRunningProcesses();

    std::vector<Process*>::iterator Begin() { return processes.begin(); }
    std::vector<Process*>::iterator End() { return processes.end(); }

  private:
    System& system;

    std::mutex mutex;
    std::vector<Process*> processes;
};

} // namespace hydra::horizon::kernel
