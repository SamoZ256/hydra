#pragma once

#define DEBUGGER_INSTANCE debugger::Debugger::GetInstance()

namespace hydra::hw::tegra_x1::cpu {
class IThread;
}

namespace hydra::debugger {

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
    friend class Debugger;

  public:
    Thread(const std::string_view name_,
           hw::tegra_x1::cpu::IThread* guest_thread_ = nullptr);

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
    hw::tegra_x1::cpu::IThread* guest_thread;

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
    void RegisterSymbol(const Symbol& symbol) {
        std::unique_lock lock(mutex);
        symbols.push_back(symbol);
    }

    std::string FindSymbol(vaddr_t addr) {
        std::unique_lock lock(mutex);
        for (const auto& symbol : symbols) {
            if (symbol.guest_mem_range.Contains(addr))
                return symbol.name;
        }

        return "";
    }

  private:
    std::mutex mutex;
    std::vector<Symbol> symbols;
};

class Debugger {
  public:
    static Debugger& GetInstance() {
        static Debugger s_instance;
        return s_instance;
    }

    Debugger();
    ~Debugger();

    void Enable();
    void Disable();

    void RegisterThisThread(const std::string_view name,
                            hw::tegra_x1::cpu::IThread* guest_thread = nullptr);
    void UnregisterThisThread();

    void BreakOnThisThread(const std::string_view reason);

    SymbolTable& GetModuleTable() { return module_table; }
    SymbolTable& GetFunctionTable() { return function_table; }

    // API
    void Lock() { thread_mutex.lock(); }
    void Unlock() { thread_mutex.unlock(); }

    usize GetThreadCount() const { return threads.size(); }
    Thread& GetThread(const u32 index) {
        // TODO: not the best way to index into a map
        auto it = threads.begin();
        std::advance(it, index);
        return it->second;
    }

  private:
    std::mutex thread_mutex;
    std::map<std::thread::id, Thread> threads;

    SymbolTable module_table;
    SymbolTable function_table;

    void LogOnThisThread(const LogMessage& msg);

    // Helpers
    StackTrace GetStackTrace(Thread& thread);
};

} // namespace hydra::debugger
