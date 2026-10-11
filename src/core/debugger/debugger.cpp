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

ResolvedStackFrame StackFrame::resolve() const {
    switch (type) {
    case StackFrameType::Host:
        // TODO
        return {.module = "libhydra.dylib", .function = "", .addr = addr};
    case StackFrameType::Guest: {
        const auto& module = debugger->getModuleTable().findSymbol(addr);
        const auto& function = debugger->getFunctionTable().findSymbol(addr);
        return {.module = module, .function = function, .addr = addr};
    }
    }
}

Thread::Thread(const std::string_view name_) : name{name_} {
    // TODO: make this configurable
    messages.resize(256);
}

void Thread::log(const Message& msg) {
    std::unique_lock lock(msg_mutex);
    messages[(msg_tail + msg_count) % messages.size()] = msg;
    if (msg_count < messages.size())
        msg_count++;
    else
        msg_tail = (msg_tail + 1) % messages.size();
}

Debugger::Debugger(const std::string_view name_,
                   horizon::kernel::Process* process_)
    : name{name_}, process{process_} {
    if (process == nullptr)
        return;

    target.emplace(*process);
    target->setHaltStateObserver(
        [this](thread_id_t id, const std::optional<StopInfo>& stop) {
            onHaltStateChanged(id, stop);
        });
}

void Debugger::registerThisThread(const std::string_view thread_name) {
    std::unique_lock lock(mutex);
    ASSERT(threads.try_emplace(std::this_thread::get_id(), thread_name).second,
           Debugger, "Failed to register thread");
}

void Debugger::unregisterThisThread() {
    GET_THIS_THREAD();
    threads.erase(it);
}

void Debugger::registerGuestThreadForThisThread(
    horizon::kernel::GuestThread* guest_thread) {
    {
        GET_THIS_THREAD();
        thread.guest_thread = guest_thread;
    }

    if (!target.has_value())
        return;

    const auto id = target->registerThread(guest_thread);

    GET_THIS_THREAD();
    thread.target_thread_id = id;
}

void Debugger::unregisterGuestThreadForThisThread() {
    thread_id_t id;
    {
        GET_THIS_THREAD();
        thread.guest_thread = nullptr;
        id = thread.target_thread_id;
        thread.target_thread_id = invalid<thread_id_t>();
    }

    if (target.has_value() && id != invalid<thread_id_t>())
        target->unregisterThread(id);
}

void Debugger::activateGdbServer() { gdb_server.emplace(*this); }

void Debugger::notifyHalted(horizon::kernel::GuestThread* thread,
                            const hw::tegra_x1::cpu::HaltInfo& halt) {
    if (thread == nullptr || !target.has_value())
        return;

    target->notifyHalted(thread, halt);
}

void Debugger::logOnThisThread(const LogMessage& msg) {
    GET_THIS_THREAD();
    lock.unlock();
    auto stack_trace = getStackTrace(thread);
    lock.lock();
    thread.log({.log = msg, .stack_trace = stack_trace});
}

void Debugger::breakOnThisThreadImpl(const std::string_view reason) {
    LOG_ERROR(Debugger, "BREAK ({})", reason);

    horizon::kernel::GuestThread* guest_thread;
    {
        GET_THIS_THREAD();
        thread.status = ThreadStatus::Break;
        thread.break_reason = reason;
        guest_thread = thread.guest_thread;
    }

    if (guest_thread != nullptr)
        notifyHalted(guest_thread,
                     {.reason = hw::tegra_x1::cpu::HaltReason::Fault});
    else if (target.has_value())
        target->pauseAll();
}

StackTrace Debugger::getStackTrace(Thread& thread) {
    StackTrace stack_trace;

    // Host
    // TODO

    // Guest
    if (auto guest_thread = thread.guest_thread) {
        guest_thread->getThread()->getStackTrace([&](vaddr_t addr) {
            stack_trace.frames.emplace_back(this, StackFrameType::Guest, addr);
        });
    }

    return stack_trace;
}

void Debugger::onHaltStateChanged(thread_id_t id,
                                  const std::optional<StopInfo>& stop) {
    {
        std::unique_lock lock(mutex);
        auto* thread = findThreadByTargetId(id);
        if (thread == nullptr)
            return;

        if (stop) {
            thread->status = ThreadStatus::Break;
            if (thread->break_reason.empty())
                thread->break_reason = fmt::format("{}", stop->reason);
        } else {
            thread->status = ThreadStatus::Running;
            thread->break_reason.clear();
        }
    }

    if (!stop)
        return;

    switch (stop->reason) {
    case hw::tegra_x1::cpu::HaltReason::BrkInstruction:
    case hw::tegra_x1::cpu::HaltReason::Fault:
        LOG_ERROR(Debugger, "Thread halted ({})", stop->reason);
        break;
    case hw::tegra_x1::cpu::HaltReason::Breakpoint:
    case hw::tegra_x1::cpu::HaltReason::StepComplete:
    case hw::tegra_x1::cpu::HaltReason::Watchpoint:
        LOG_INFO(Debugger, "Thread halted ({})", stop->reason);
        break;
    }
}

Thread* Debugger::findThreadByTargetId(thread_id_t id) {
    for (auto& [_, thread] : threads) {
        if (thread.target_thread_id == id)
            return &thread;
    }

    return nullptr;
}

} // namespace hydra::debugger
