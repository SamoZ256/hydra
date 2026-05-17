#include "core/debugger/debugger.hpp"

#include "core/debugger/gdb_server.hpp"
#include "core/horizon/kernel/guest_thread.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

#define GET_THIS_THREAD()                                                      \
    std::unique_lock lock(mutex);                                              \
    const auto thread_id = std::this_thread::get_id();                         \
    auto it = threads.find(thread_id);                                         \
    ASSERT_DEBUG(it != threads.end(), Debugger,                                \
                 "Thread {:016x} not registered",                              \
                 std::bit_cast<u64>(thread_id));                               \
    [[maybe_unused]] auto& thread = it->second;

namespace hydra::debugger {

ResolvedStackFrame StackFrame::Resolve() const {
    switch (type) {
    case StackFrameType::Host:
        // TODO
        return {"libhydra.dylib", "", addr};
    case StackFrameType::Guest: {
        const auto& module = debugger->GetModuleTable().FindSymbol(addr);
        const auto& function = debugger->GetFunctionTable().FindSymbol(addr);
        return {module, function, addr};
    }
    }
}

Thread::Thread(const std::string_view name_) : name{name_} {
    // TODO: make this configurable
    messages.resize(256);
}

void Thread::Log(const Message& msg) {
    std::unique_lock lock(msg_mutex);
    messages[(msg_tail + msg_count) % messages.size()] = msg;
    if (msg_count < messages.size())
        msg_count++;
    else
        msg_tail = (msg_tail + 1) % messages.size();
}

Debugger::~Debugger() {
    if (gdb_server)
        delete gdb_server;
}

void Debugger::RegisterThisThread(const std::string_view thread_name) {
    std::unique_lock lock(mutex);
    ASSERT(threads.try_emplace(std::this_thread::get_id(), thread_name).second,
           Debugger, "Failed to register thread");
}

void Debugger::UnregisterThisThread() {
    GET_THIS_THREAD();
    threads.erase(it);
}

void Debugger::RegisterGuestThreadForThisThread(
    horizon::kernel::GuestThread* guest_thread) {
    GET_THIS_THREAD();
    thread.guest_thread = guest_thread;

    if (gdb_server)
        gdb_server->RegisterThread(thread);
}

void Debugger::UnregisterGuestThreadForThisThread() {
    GET_THIS_THREAD();
    thread.guest_thread = nullptr;
}

void Debugger::ActivateGdbServer(System& system) {
    gdb_server = new GdbServer(system, *this);
}

void Debugger::NotifySupervisorPaused(horizon::kernel::GuestThread* thread,
                                      Signal signal) {
    if (gdb_server)
        gdb_server->NotifySupervisorPaused(thread, signal);
}

void Debugger::BreakpointHit(horizon::kernel::GuestThread* thread) {
    if (gdb_server)
        gdb_server->BreakpointHit(thread);
}

void Debugger::LogOnThisThread(const LogMessage& msg) {
    GET_THIS_THREAD();
    lock.unlock();
    auto stack_trace = GetStackTrace(thread);
    lock.lock();
    thread.Log({msg, stack_trace});
}

void Debugger::BreakOnThisThreadImpl(const std::string_view reason) {
    LOG_ERROR(Debugger, "BREAK ({})", reason);

    horizon::kernel::GuestThread* guest_thread;
    {
        GET_THIS_THREAD();
        thread.status = ThreadStatus::Break;
        thread.break_reason = reason;
        guest_thread = thread.guest_thread;
    }

    process->SupervisorPause();
    NotifySupervisorPaused(
        guest_thread,
        Signal::SigHup); // TODO: make the signal configurable
}

StackTrace Debugger::GetStackTrace(Thread& thread) {
    StackTrace stack_trace;

    // Host
    // TODO

    // Guest
    if (auto guest_thread = thread.guest_thread) {
        guest_thread->GetThread()->GetStackTrace([&](vaddr_t addr) {
            stack_trace.frames.emplace_back(this, StackFrameType::Guest, addr);
        });
    }

    return stack_trace;
}

} // namespace hydra::debugger
