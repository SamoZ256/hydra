#include "core/debugger/debugger.hpp"

#include <future>

#include "core/hw/tegra_x1/cpu/thread.hpp"

#define GET_THIS_THREAD()                                                      \
    std::unique_lock lock(thread_mutex);                                       \
    const auto thread_id = std::this_thread::get_id();                         \
    auto it = threads.find(thread_id);                                         \
    /* We need to do a regular assert so as to avoid infinite log recursion */ \
    /* TODO: assert_debug */                                                   \
    assert(it != threads.end() && "Thread {} not registered");                 \
    auto& thread = it->second;

namespace hydra::debugger {

ResolvedStackFrame StackFrame::Resolve() const {
    switch (type) {
    case StackFrameType::Host:
        // TODO
        return {"libhydra.dylib", "", addr};
    case StackFrameType::Guest: {
        const auto& module =
            DEBUGGER_INSTANCE.GetModuleTable().FindSymbol(addr);
        const auto& function =
            DEBUGGER_INSTANCE.GetFunctionTable().FindSymbol(addr);
        return {module, function, addr};
    }
    }
}

Thread::Thread(const std::string_view name_,
               hw::tegra_x1::cpu::IThread* guest_thread_)
    : name{name_}, guest_thread{guest_thread_} {
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

Debugger::Debugger() { RegisterThisThread("Main"); }

Debugger::~Debugger() { UnregisterThisThread(); }

void Debugger::Enable() {
    LOGGER_INSTANCE.InstallCallback(
        [this](const LogMessage& msg) { LogOnThisThread(msg); });
}

void Debugger::Disable() {
    std::unique_lock lock(thread_mutex);
    LOGGER_INSTANCE.UninstallCallback();
    for (auto& [id, thread] : threads) {
        std::unique_lock lock(thread.msg_mutex);
        thread.messages.clear();
    }
}

void Debugger::RegisterThisThread(const std::string_view name,
                                  hw::tegra_x1::cpu::IThread* guest_thread) {
    std::unique_lock lock(thread_mutex);
    threads.try_emplace(std::this_thread::get_id(), name, guest_thread);
}

void Debugger::UnregisterThisThread() {
    GET_THIS_THREAD();
    threads.erase(it);
}

void Debugger::BreakOnThisThread(const std::string_view reason) {
    {
        GET_THIS_THREAD();
        thread.status = ThreadStatus::Break;
        thread.break_reason = reason;
    }

    // TODO: abort after the debugger is closed?
    std::promise<void> promise;
    auto future = promise.get_future();
    future.wait();
}

void Debugger::LogOnThisThread(const LogMessage& msg) {
    GET_THIS_THREAD();
    lock.unlock();
    auto stack_trace = GetStackTrace(thread);
    lock.lock();
    thread.Log({msg, stack_trace});
}

StackTrace Debugger::GetStackTrace(Thread& thread) {
    StackTrace stack_trace;

    // Host
    // TODO

    // Guest
    if (auto guest_thread = thread.guest_thread) {
        guest_thread->GetStackTrace([&](vaddr_t addr) {
            stack_trace.frames.emplace_back(StackFrameType::Guest, addr);
        });
    }

    return std::move(stack_trace);
}

} // namespace hydra::debugger
