#include "core/debugger/debugger.hpp"

#include "core/debugger/gdb_server.hpp"
#include "core/horizon/kernel/guest_thread.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

#include <limits>

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

std::expected<Listing, DisassemblyError>
Debugger::disassemble(const DisassemblyRequest& request) {
    if (!target.has_value())
        return std::unexpected(DisassemblyError::NoTarget);

    if (request.addr % INSTRUCTION_SIZE != 0)
        return std::unexpected(DisassemblyError::Unaligned);

    std::scoped_lock lock(disassembler_mutex);
    if (!disassembler)
        disassembler = createDisassembler();
    if (!disassembler)
        return std::unexpected(DisassemblyError::Unavailable);

    const auto max_count =
        std::min<u64>(MAX_LISTING_INSTRUCTIONS,
                      (std::numeric_limits<vaddr_t>::max() - request.addr) /
                          INSTRUCTION_SIZE);
    const auto count = std::min<u64>(request.count, max_count);
    const auto code = readCode(request.addr, count * INSTRUCTION_SIZE);
    if (code.empty())
        return std::unexpected(DisassemblyError::Unmapped);

    const auto breakpoints = target->getBreakpoints();
    auto listing = buildListing(request.addr,
                                disassembler->disassemble(request.addr, code),
                                {.module_table = module_table,
                                 .function_table = function_table,
                                 .breakpoints = breakpoints,
                                 .pc = request.pc});
    listing.truncated =
        request.count > max_count || code.size() < count * INSTRUCTION_SIZE;
    return listing;
}

std::expected<Listing, DisassemblyError>
Debugger::disassembleFunction(vaddr_t addr, std::optional<vaddr_t> pc) {
    const auto function = function_table.tryResolve(addr);
    if (!function)
        return std::unexpected(DisassemblyError::NoFunction);

    const auto count =
        std::clamp<u64>(function->range.getSize() / INSTRUCTION_SIZE, 1,
                        MAX_LISTING_INSTRUCTIONS);
    return disassemble({.addr = function->range.getBegin(),
                        .count = static_cast<u32>(count),
                        .pc = pc});
}

void Debugger::logOnThisThread(const LogMessage& msg) {
    GET_THIS_THREAD();
    lock.unlock();
    auto stack_trace = getStackTrace(thread);
    lock.lock();
    thread.log({.log = msg, .stack_trace = stack_trace});
}

std::vector<u8> Debugger::readCode(vaddr_t addr, usize size) {
    constexpr auto PAGE_SIZE = hw::tegra_x1::cpu::GUEST_PAGE_SIZE;

    std::vector<u8> code(size);
    usize read = 0;
    while (read < size) {
        const auto crnt = addr + read;
        const auto page_end = (crnt / PAGE_SIZE + 1) * PAGE_SIZE;
        const auto chunk = std::min<usize>(size - read, page_end - crnt);
        if (!target->tryReadMemory(crnt, std::span(code).subspan(read, chunk)))
            break;

        read += chunk;
    }

    code.resize(read - read % INSTRUCTION_SIZE);
    return code;
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
