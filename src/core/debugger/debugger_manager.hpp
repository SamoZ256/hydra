#pragma once

#include "core/debugger/debugger.hpp"

#define DEBUGGER_MANAGER_INSTANCE debugger::DebuggerManager::getInstance()
#define GET_CURRENT_PROCESS_DEBUGGER()                                         \
    DEBUGGER_MANAGER_INSTANCE.getDebuggerForCurrentProcess()

namespace hydra::debugger {

// TODO: sort out mutex locking
class DebuggerManager {
  public:
    static DebuggerManager& getInstance();

    DebuggerManager();
    ~DebuggerManager();

    void attachDebugger(hydra::horizon::kernel::Process* process,
                        const std::string_view name);
    void detachDebugger(hydra::horizon::kernel::Process* process);

    Debugger& getDebugger(hydra::horizon::kernel::Process* process);
    Debugger& getDebuggerForCurrentProcess();

    // API
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    usize getDebuggerCount() const { return debuggers.size() + 1; }
    Debugger& getDebugger(const u32 index) {
        if (index == 0)
            return hydra_debugger;

        // TODO: not the best way to index into a map
        auto it = debuggers.begin();
        std::advance(it, index - 1);
        return it->second;
    }

  private:
    std::mutex mutex;
    Debugger hydra_debugger;
    std::map<hydra::horizon::kernel::Process*, Debugger> debuggers;
};

} // namespace hydra::debugger
