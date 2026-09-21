#pragma once

#include "core/debugger/gdb_server.hpp"

namespace hydra {
class System;
}

namespace hydra::horizon::kernel {
class GuestThread;
class Process;
} // namespace hydra::horizon::kernel

namespace hydra::horizon::filesystem {
class IFile;
} // namespace hydra::horizon::filesystem

#define DEBUGGER_ASSERT(condition, c, f, ...)                                  \
    if (!(condition)) {                                                        \
        /* TODO: log class? */                                                 \
        GET_CURRENT_PROCESS_DEBUGGER().breakOnThisThread(                      \
            f ZTD_PASS_VA_ARGS(__VA_ARGS__));                                  \
    }

#ifdef HYDRA_DEBUG
#define DEBUGGER_ASSERT_DEBUG(condition, c, ...)                               \
    DEBUGGER_ASSERT(condition, c, __VA_ARGS__)
#else
#define DEBUGGER_ASSERT_DEBUG(condition, c, ...)                               \
    if (condition) {                                                           \
    }
#endif

namespace hydra::debugger {

class Debugger;

struct ResolvedStackFrame {
    std::string module;
    std::string function;
    u64 addr;
};

enum class StackFrameType {
    Host,
    Guest,
};

struct StackFrame {
    Debugger* debugger;
    StackFrameType type;
    u64 addr;

    ResolvedStackFrame resolve() const;
};

struct StackTrace {
    std::vector<StackFrame> frames;
};

struct Message {
    LogMessage log;
    StackTrace stack_trace;
};

enum class ThreadStatus {
    Running,
    Break,
};

class Thread {
    friend class GdbServer;
    friend class Debugger;

  public:
    explicit Thread(const std::string_view name_);

    // API
    void lock() { msg_mutex.lock(); }
    void unlock() { msg_mutex.unlock(); }

    const std::string& getName() const { return name; }
    ThreadStatus getStatus() const { return status; }
    const std::string& getBreakReason() const { return break_reason; }
    usize getMessageCount() const { return msg_count; }
    const Message& getMessage(const u32 index) const {
        return messages[(msg_tail + index) % messages.size()];
    }

  private:
    std::string name;
    horizon::kernel::GuestThread* guest_thread{nullptr};

    ThreadStatus status{ThreadStatus::Running};
    std::string break_reason;

    std::mutex msg_mutex;
    std::vector<Message> messages;
    u32 msg_tail{0};
    usize msg_count{0};

    void log(const Message& msg);

  public:
    SETTER(guest_thread, setGuestThread);
};

struct Symbol {
    std::string name;
    ztd::Range<vaddr_t> guest_mem_range;
};

class SymbolTable {
  public:
    void registerSymbol(const Symbol& symbol) { symbols.push_back(symbol); }

    std::string findSymbol(vaddr_t addr) {
        for (const auto& symbol : symbols) {
            if (symbol.guest_mem_range.contains(addr))
                return symbol.name;
        }

        return "";
    }

  private:
    std::vector<Symbol> symbols;

  public:
    CONST_REF_GETTER(symbols, getSymbols);
};

class Debugger {
    friend class GdbServer;
    friend class DebuggerManager;

  public:
    Debugger(const std::string_view name_, horizon::kernel::Process* process_)
        : name{name_}, process{process_} {}
    ~Debugger() noexcept = default;

    void registerExecutable(const std::string_view exe_name,
                            horizon::filesystem::IFile* executable) {
        executables.emplace(exe_name, executable);
    }

    void registerThisThread(const std::string_view thread_name);
    void unregisterThisThread();
    void registerGuestThreadForThisThread(
        horizon::kernel::GuestThread* guest_thread);
    void unregisterGuestThreadForThisThread();

    template <typename... T>
    void breakOnThisThread(fmt::format_string<T...> f, T&&... args) {
        breakOnThisThreadImpl(fmt::format(f, std::forward<T>(args)...));
    }

    SymbolTable& getModuleTable() { return module_table; }
    SymbolTable& getFunctionTable() { return function_table; }

    // GDB
    void activateGdbServer(System& system);
    void notifySupervisorPaused(horizon::kernel::GuestThread* thread,
                                Signal signal);
    void breakpointHit(horizon::kernel::GuestThread* thread);

    // API
    void lock() { mutex.lock(); }
    void unlock() { mutex.unlock(); }

    usize getThreadCount() const { return threads.size(); }
    Thread& getThread(const u32 index) {
        // TODO: not the best way to index into a map
        auto it = threads.begin();
        std::advance(it, index);
        return it->second;
    }

  private:
    std::string name;
    horizon::kernel::Process* process;

    std::mutex mutex;
    std::map<std::thread::id, Thread> threads;

    std::map<std::string, horizon::filesystem::IFile*> executables;
    SymbolTable module_table;
    SymbolTable function_table;

    std::optional<GdbServer> gdb_server;

    void logOnThisThread(const LogMessage& msg);
    void breakOnThisThreadImpl(const std::string_view reason);

    // Helpers
    StackTrace getStackTrace(Thread& thread);

  public:
    CONST_REF_GETTER(name, getName);
};

} // namespace hydra::debugger
