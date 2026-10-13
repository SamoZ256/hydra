#pragma once

#include "core/debugger/gdb_server.hpp"
#include "core/debugger/listing.hpp"
#include "core/debugger/symbol_table.hpp"
#include "core/debugger/target.hpp"

namespace hydra::horizon::kernel {
class GuestThread;
class Process;
} // namespace hydra::horizon::kernel

namespace hydra::horizon::filesystem {
class IFile;
} // namespace hydra::horizon::filesystem

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
    thread_id_t target_thread_id{invalid<thread_id_t>()};

    std::mutex msg_mutex;
    std::vector<Message> messages;
    u32 msg_tail{0};
    usize msg_count{0};

    void log(const Message& msg);

  public:
    SETTER(guest_thread, setGuestThread);
};

class Debugger {
    friend class DebuggerManager;

  public:
    Debugger(const std::string_view name_,
             horizon::kernel::Process* process_);
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

    SymbolTable& getModuleTable() { return module_table; }
    SymbolTable& getFunctionTable() { return function_table; }

    // Disassembly
    std::expected<Listing, DisassemblyError>
    disassemble(const DisassemblyRequest& request);
    std::expected<Listing, DisassemblyError>
    disassembleFunction(vaddr_t addr, std::optional<vaddr_t> pc = std::nullopt);

    // Execution
    void activateGdbServer();

    bool hasTarget() const { return target.has_value(); }
    Target& getTarget() {
        ASSERT_DEBUG(target.has_value(), Debugger, "No target");
        return target.value();
    }

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

    std::optional<Target> target;
    std::optional<GdbServer> gdb_server;

    std::mutex disassembler_mutex;
    std::unique_ptr<IDisassembler> disassembler;

    void logOnThisThread(const LogMessage& msg);

    // Helpers
    StackTrace getStackTrace(Thread& thread);
    std::vector<u8> readCode(vaddr_t addr, usize size);
    void onHaltStateChanged(thread_id_t id,
                            const std::optional<StopInfo>& stop);
    Thread* findThreadByTargetId(thread_id_t id);

  public:
    CONST_REF_GETTER(name, getName);
    CONST_REF_GETTER(executables, getExecutables);
};

} // namespace hydra::debugger
