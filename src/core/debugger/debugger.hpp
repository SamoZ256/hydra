#pragma once

namespace hydra::horizon::kernel {
class GuestThread;
class Process;
} // namespace hydra::horizon::kernel

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
    Thread(const std::string_view name_,
           horizon::kernel::GuestThread* guest_thread_ = nullptr);

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
    horizon::kernel::GuestThread* guest_thread;

    ThreadStatus status{ThreadStatus::Running};
    std::string break_reason;

    std::mutex msg_mutex;
    std::vector<Message> messages;
    u32 msg_tail{0};
    usize msg_count{0};

    void Log(const Message& msg);
};

struct Symbol {
    std::string name;
    range<vaddr_t> guest_mem_range;
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

    void
    RegisterThisThread(const std::string_view name,
                       horizon::kernel::GuestThread* guest_thread = nullptr);
    void UnregisterThisThread();

    void BreakOnThisThread(const std::string_view reason);

    SymbolTable& GetModuleTable() { return module_table; }
    SymbolTable& GetFunctionTable() { return function_table; }

    // GDB
    void ActivateGdbServer();

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

    SymbolTable module_table;
    SymbolTable function_table;

    GdbServer* gdb_server{nullptr};

    void LogOnThisThread(const LogMessage& msg);

    // Helpers
    StackTrace GetStackTrace(Thread& thread);

  public:
    CONST_REF_GETTER(name, GetName);
};

} // namespace hydra::debugger
