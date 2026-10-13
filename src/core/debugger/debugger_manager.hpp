#pragma once

#define DEBUGGER_MANAGER_INSTANCE debugger::DebuggerManager::getInstance()
#define GET_CURRENT_PROCESS_DEBUGGER()                                         \
    DEBUGGER_MANAGER_INSTANCE.getDebuggerForCurrentProcess()

#ifdef HYDRA_DEBUGGER_ENABLED

#include "core/debugger/debugger.hpp"

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

#else

namespace hydra::horizon::kernel {
class GuestThread;
class Process;
} // namespace hydra::horizon::kernel

namespace hydra::horizon::filesystem {
class IFile;
} // namespace hydra::horizon::filesystem

namespace hydra::debugger {

struct Symbol {
    std::string name;
    ztd::Range<vaddr_t> guest_mem_range;
};

class SymbolTable {
  public:
    void registerSymbol([[maybe_unused]] const Symbol& symbol) {}
    void registerSymbols([[maybe_unused]] std::span<const Symbol> symbols) {}
};

class Debugger {
  public:
    void registerExecutable([[maybe_unused]] const std::string_view exe_name,
                            [[maybe_unused]] horizon::filesystem::IFile* file) {
    }

    void registerThisThread([[maybe_unused]] const std::string_view name) {}
    void unregisterThisThread() {}
    void registerGuestThreadForThisThread(
        [[maybe_unused]] horizon::kernel::GuestThread* guest_thread) {}
    void unregisterGuestThreadForThisThread() {}

    SymbolTable& getModuleTable() { return symbol_table; }
    SymbolTable& getFunctionTable() { return symbol_table; }

  private:
    SymbolTable symbol_table;
};

class DebuggerManager {
  public:
    static DebuggerManager& getInstance() {
        static DebuggerManager instance;
        return instance;
    }

    void attachDebugger([[maybe_unused]] horizon::kernel::Process* process,
                        [[maybe_unused]] const std::string_view name) {}
    void detachDebugger([[maybe_unused]] horizon::kernel::Process* process) {}

    Debugger& getDebugger([[maybe_unused]] horizon::kernel::Process* process) {
        return debugger;
    }
    Debugger& getDebuggerForCurrentProcess() { return debugger; }

  private:
    Debugger debugger;
};

} // namespace hydra::debugger

#endif
