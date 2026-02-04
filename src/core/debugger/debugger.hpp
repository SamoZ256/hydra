#pragma once

#include "core/debugger/const.hpp"

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
        GET_CURRENT_PROCESS_DEBUGGER().BreakOnThisThread(                      \
            f PASS_VA_ARGS(__VA_ARGS__));                                      \
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

class GdbServer;
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

    ResolvedStackFrame Resolve() const;
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
    Thread(const std::string_view name_);

    // API
    void Lock() { msg_mutex.lock(); }
    void Unlock() { msg_mutex.unlock(); }

    const std::string& GetName() const { return name; }
    ThreadStatus GetStatus() const { return status; }
    const std::string& GetBreakReason() const { return break_reason; }
    usize GetMessageCount() const { return msg_count; }
    const Message& GetMessage(const u32 index) const {
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

    void Log(const Message& msg);

  public:
    SETTER(guest_thread, SetGuestThread);
};

struct Symbol {
    std::string name;
    Range<vaddr_t> guest_mem_range;
};

class SymbolTable {
  public:
    void RegisterSymbol(const Symbol& symbol) { symbols.push_back(symbol); }

    std::string FindSymbol(vaddr_t addr) {
        for (const auto& symbol : symbols) {
            if (symbol.guest_mem_range.Contains(addr))
                return symbol.name;
        }

        return "";
    }

  private:
    std::vector<Symbol> symbols;

  public:
    CONST_REF_GETTER(symbols, GetSymbols);
};

class Debugger {
    friend class GdbServer;
    friend class DebuggerManager;

  public:
    Debugger(const std::string_view name_, horizon::kernel::Process* process_)
        : name{name_}, process{process_} {}
    ~Debugger();

    void RegisterExecutable(const std::string_view name,
                            horizon::filesystem::IFile* executable) {
        executables.emplace(name, executable);
    }

    void RegisterThisThread(const std::string_view name);
    void UnregisterThisThread();
    void RegisterGuestThreadForThisThread(
        horizon::kernel::GuestThread* guest_thread);
    void UnregisterGuestThreadForThisThread();

    template <typename... T>
    void BreakOnThisThread(fmt::format_string<T...> f, T&&... args) {
        BreakOnThisThreadImpl(fmt::format(f, std::forward<T>(args)...));
    }

    SymbolTable& GetModuleTable() { return module_table; }
    SymbolTable& GetFunctionTable() { return function_table; }

    // GDB
    void ActivateGdbServer();
    void NotifySupervisorPaused(horizon::kernel::GuestThread* thread,
                                Signal signal);
    void BreakpointHit(horizon::kernel::GuestThread* thread);

    // API
    void Lock() { mutex.lock(); }
    void Unlock() { mutex.unlock(); }

    usize GetThreadCount() const { return threads.size(); }
    Thread& GetThread(const u32 index) {
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

    GdbServer* gdb_server{nullptr};

    void LogOnThisThread(const LogMessage& msg);
    void BreakOnThisThreadImpl(const std::string_view reason);

    // Helpers
    StackTrace GetStackTrace(Thread& thread);

  public:
    CONST_REF_GETTER(name, GetName);
};

} // namespace hydra::debugger
