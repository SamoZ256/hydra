#pragma once

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {

class Process;

class ProcessManager {
  public:
    explicit ProcessManager(System& system_) : system{system_} {}
    ~ProcessManager();

    Process* createProcess(const std::string_view name);
    void destroyProcess(Process* process);

    bool hasRunningProcesses();

    std::vector<Process*>::iterator begin() { return processes.begin(); }
    std::vector<Process*>::iterator end() { return processes.end(); }

  private:
    System& system;

    std::mutex mutex;
    std::vector<Process*> processes;
};

} // namespace hydra::horizon::kernel
