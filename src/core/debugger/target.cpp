#include "core/debugger/target.hpp"

#include "core/horizon/kernel/guest_thread.hpp"
#include "core/horizon/kernel/process.hpp"
#include "core/hw/tegra_x1/cpu/cpu.hpp"
#include "core/hw/tegra_x1/cpu/mmu.hpp"
#include "core/system.hpp"

namespace hydra::debugger {

namespace {

constexpr u32 BRK = 0xd4200000;
constexpr u32 BRK_MASK = 0xffe0001f;
constexpr usize NATIVE_BREAKPOINT_SLOTS = 16;
constexpr i64 STOP_POLL_INTERVAL_NS = 1000ll * 1000ll;

} // namespace

using hw::tegra_x1::cpu::HaltReason;

Target::Target(horizon::kernel::Process& process_)
    : process{process_}, native_breakpoints{process_.getSystem()
                                                .getCpu()
                                                .getFeatures()
                                                .supports_native_breakpoints},
      synchronous_single_step{process_.getSystem()
                                  .getCpu()
                                  .getFeatures()
                                  .supports_synchronous_single_step} {}

thread_id_t Target::registerThread(horizon::kernel::GuestThread* guest_thread) {
    std::scoped_lock lock(mutex);

    const auto id = next_thread_id++;
    auto* cpu_thread = guest_thread->getThread();
    threads.emplace(id, ThreadEntry{.guest_thread = guest_thread,
                                    .cpu_thread = cpu_thread});

    if (native_breakpoints) {
        for (const auto& [addr, _] : breakpoints)
            cpu_thread->insertBreakpoint(addr);
    }

    return id;
}

void Target::unregisterThread(thread_id_t id) {
    std::scoped_lock lock(mutex);
    threads.erase(id);
}

std::vector<thread_id_t> Target::getThreadIds() const {
    std::scoped_lock lock(mutex);

    std::vector<thread_id_t> ids;
    ids.reserve(threads.size());
    for (const auto& [id, _] : threads)
        ids.push_back(id);

    return ids;
}

bool Target::hasThread(thread_id_t id) const {
    std::scoped_lock lock(mutex);
    return threads.contains(id);
}

std::optional<HaltReason> Target::getHaltReason(thread_id_t id) const {
    std::scoped_lock lock(mutex);

    const auto* entry = findEntry(id);
    if (entry == nullptr)
        return std::nullopt;

    return entry->halt_reason;
}

void Target::pauseAll() {
    if (!isProcessRunning())
        return;

    process.supervisorPause();
}

void Target::resumeAll() {
    if (!isProcessRunning())
        return;

    std::vector<thread_id_t> resumed;
    {
        std::scoped_lock lock(mutex);
        stop_reported = false;
        for (auto& [id, entry] : threads) {
            if (entry.halt_reason) {
                entry.halt_reason = std::nullopt;
                resumed.push_back(id);
            }
        }
    }

    process.supervisorResume();

    if (halt_state_observer) {
        for (const auto id : resumed)
            halt_state_observer(id, std::nullopt, false);
    }
}

bool Target::resumeThread(thread_id_t id) {
    horizon::kernel::GuestThread* guest_thread;
    {
        std::scoped_lock lock(mutex);
        auto* entry = findEntry(id);
        if (entry == nullptr)
            return false;

        entry->halt_reason = std::nullopt;
        stop_reported = false;
        guest_thread = entry->guest_thread;
    }

    guest_thread->supervisorResume();

    if (halt_state_observer)
        halt_state_observer(id, std::nullopt, false);

    return true;
}

bool Target::stepThread(thread_id_t id) {
    horizon::kernel::GuestThread* guest_thread;
    hw::tegra_x1::cpu::IThread* cpu_thread;
    {
        std::scoped_lock lock(mutex);
        auto* entry = findEntry(id);
        if (entry == nullptr || !isSteppable(*entry))
            return false;

        entry->halt_reason = std::nullopt;
        stop_reported = false;
        guest_thread = entry->guest_thread;
        cpu_thread = entry->cpu_thread;
    }

    if (halt_state_observer)
        halt_state_observer(id, std::nullopt, false);

    cpu_thread->singleStep();

    if (synchronous_single_step) {
        bool already_reported;
        {
            std::scoped_lock lock(mutex);
            already_reported = stop_reported;
        }

        if (!already_reported)
            notifyHalted(guest_thread, HaltReason::StepComplete);
    } else {
        guest_thread->supervisorResume();
    }

    return true;
}

void Target::stop() { process.stop(); }

void Target::notifyHalted(horizon::kernel::GuestThread* guest_thread,
                          HaltReason reason) {
    std::optional<thread_id_t> id;
    bool first_stop = false;
    {
        std::scoped_lock lock(mutex);

        auto* cpu_thread = guest_thread->getThread();
        if (cpu_thread != nullptr &&
            isStaleBreakpointTrap(reason, cpu_thread->getState().pc)) {
            guest_thread->supervisorResume();
            return;
        }

        if (reason == HaltReason::BrkInstruction &&
            breakpoints.contains(cpu_thread->getState().pc))
            reason = HaltReason::Breakpoint;

        for (auto& [entry_id, entry] : threads) {
            if (entry.guest_thread == guest_thread) {
                id = entry_id;
                entry.halt_reason = reason;
                break;
            }
        }

        if (!id)
            return;

        first_stop = !stop_reported;
        stop_reported = true;
    }

    process.supervisorPause();

    if (halt_state_observer)
        halt_state_observer(id.value(), reason, first_stop);
}

std::optional<StoppedThread>
Target::tryAcquireStoppedThread(thread_id_t id, i64 timeout_ns) {
    const auto deadline =
        std::chrono::steady_clock::now() + std::chrono::nanoseconds(timeout_ns);

    while (true) {
        {
            std::unique_lock<std::mutex> lock(mutex);
            auto* entry = findEntry(id);
            if (entry == nullptr)
                return std::nullopt;

            if (isStateAccessible(*entry))
                return StoppedThread(std::move(lock), entry->cpu_thread);
        }

        if (std::chrono::steady_clock::now() >= deadline)
            return std::nullopt;

        std::this_thread::sleep_for(
            std::chrono::nanoseconds(STOP_POLL_INTERVAL_NS));
    }
}

bool Target::tryReadMemory(vaddr_t addr, std::span<u8> out_data) const {
    std::scoped_lock lock(mutex);

    if (!tryReadMemoryRaw(addr, out_data))
        return false;

    readBreakpointShadow(addr, out_data);
    return true;
}

bool Target::tryWriteMemory(vaddr_t addr, std::span<const u8> data) {
    std::scoped_lock lock(mutex);

    if (!tryWriteMemoryRaw(addr, data))
        return false;

    writeBreakpointShadow(addr, data);
    notifyMemoryChanged(ztd::Range<vaddr_t>::fromSize(addr, data.size()));
    return true;
}

bool Target::insertBreakpoint(vaddr_t addr) {
    std::scoped_lock lock(mutex);

    if (breakpoints.contains(addr))
        return true;

    if (native_breakpoints) {
        if (addr == 0 || breakpoints.size() >= NATIVE_BREAKPOINT_SLOTS)
            return false;

        breakpoints.emplace(addr, Breakpoint{});
        for (const auto& [_, entry] : threads)
            entry.cpu_thread->insertBreakpoint(addr);
    } else {
        u32 original_instruction;
        if (!getMmu()->tryRead(addr, original_instruction))
            return false;

        if (!getMmu()->tryWrite(addr, BRK))
            return false;

        breakpoints.emplace(
            addr, Breakpoint{.replaced_instruction = original_instruction});
        notifyMemoryChanged(ztd::Range<vaddr_t>::fromSize(addr, sizeof(u32)));
    }

    return true;
}

bool Target::removeBreakpoint(vaddr_t addr) {
    std::scoped_lock lock(mutex);

    const auto it = breakpoints.find(addr);
    if (it == breakpoints.end())
        return false;

    return removeBreakpointLocked(it);
}

void Target::removeAllBreakpoints() {
    std::scoped_lock lock(mutex);

    for (auto it = breakpoints.begin(); it != breakpoints.end();) {
        const auto next = std::next(it);
        if (!removeBreakpointLocked(it))
            LOG_WARN(Debugger, "Failed to remove breakpoint at 0x{:08x}",
                     it->first);
        it = next;
    }
}

bool Target::hasBreakpoint(vaddr_t addr) const {
    std::scoped_lock lock(mutex);
    return breakpoints.contains(addr);
}

Target::ThreadEntry* Target::findEntry(thread_id_t id) {
    const auto it = threads.find(id);
    return it == threads.end() ? nullptr : &it->second;
}

const Target::ThreadEntry* Target::findEntry(thread_id_t id) const {
    const auto it = threads.find(id);
    return it == threads.end() ? nullptr : &it->second;
}

bool Target::isParked(const ThreadEntry& entry) const {
    return entry.guest_thread->isAtSafePoint() &&
           entry.guest_thread->isSupervisorPaused();
}

bool Target::isSteppable(const ThreadEntry& entry) const {
    if (synchronous_single_step)
        return isParked(entry);

    return entry.guest_thread->isSupervisorPaused();
}

bool Target::isStaleBreakpointTrap(HaltReason reason, vaddr_t pc) const {
    if (breakpoints.contains(pc))
        return false;

    switch (reason) {
    case HaltReason::Breakpoint:
        return true;
    case HaltReason::BrkInstruction: {
        u32 instruction;
        return !native_breakpoints && getMmu()->tryRead(pc, instruction) &&
               (instruction & BRK_MASK) != BRK;
    }
    default:
        return false;
    }
}

bool Target::isStateAccessible(const ThreadEntry& entry) const {
    if (entry.guest_thread == horizon::kernel::tls_current_thread)
        return true;

    return entry.guest_thread->isSupervisorPaused();
}

bool Target::removeBreakpointLocked(
    std::map<vaddr_t, Breakpoint>::iterator it) {
    const auto addr = it->first;
    if (native_breakpoints) {
        for (const auto& [_, entry] : threads)
            entry.cpu_thread->removeBreakpoint(addr);
    } else {
        if (!getMmu()->tryWrite(addr, it->second.replaced_instruction))
            return false;

        notifyMemoryChanged(ztd::Range<vaddr_t>::fromSize(addr, sizeof(u32)));
    }

    breakpoints.erase(it);
    return true;
}

bool Target::tryReadMemoryRaw(vaddr_t addr, std::span<u8> out_data) const {
    const auto mmu = getMmu();
    for (usize i = 0; i < out_data.size(); i++) {
        if (!mmu->tryRead(addr + i, out_data[i]))
            return false;
    }

    return true;
}

bool Target::tryWriteMemoryRaw(vaddr_t addr, std::span<const u8> data) {
    const auto mmu = getMmu();
    for (usize i = 0; i < data.size(); i++) {
        if (!mmu->tryWrite(addr + i, data[i]))
            return false;
    }

    return true;
}

void Target::readBreakpointShadow(vaddr_t addr, std::span<u8> data) const {
    if (native_breakpoints)
        return;

    const auto request = ztd::Range<vaddr_t>::fromSize(addr, data.size());
    for (const auto& [bp_addr, breakpoint] : breakpoints) {
        const auto bp_range =
            ztd::Range<vaddr_t>::fromSize(bp_addr, sizeof(u32));
        if (!request.intersects(bp_range))
            continue;

        const auto* src =
            reinterpret_cast<const u8*>(&breakpoint.replaced_instruction);
        for (u32 i = 0; i < sizeof(u32); i++) {
            const auto va = bp_addr + i;
            if (request.contains(va))
                data[va - addr] = src[i];
        }
    }
}

void Target::writeBreakpointShadow(vaddr_t addr, std::span<const u8> data) {
    if (native_breakpoints)
        return;

    const auto request = ztd::Range<vaddr_t>::fromSize(addr, data.size());
    for (auto& [bp_addr, breakpoint] : breakpoints) {
        const auto bp_range =
            ztd::Range<vaddr_t>::fromSize(bp_addr, sizeof(u32));
        if (!request.intersects(bp_range))
            continue;

        auto* dst = reinterpret_cast<u8*>(&breakpoint.replaced_instruction);
        for (u32 i = 0; i < sizeof(u32); i++) {
            const auto va = bp_addr + i;
            if (request.contains(va))
                dst[i] = data[va - addr];
        }

        getMmu()->tryWrite(bp_addr, BRK);
    }
}

void Target::notifyMemoryChanged(ztd::Range<vaddr_t> mem_range) {
    for (const auto& [_, entry] : threads)
        entry.cpu_thread->notifyMemoryChanged(mem_range);
}

hw::tegra_x1::cpu::IMmu* Target::getMmu() const { return process.getMmu(); }

bool Target::isProcessRunning() const {
    switch (process.getState()) {
    case horizon::kernel::ProcessState::Started:
    case horizon::kernel::ProcessState::DebugSuspended:
        return true;
    default:
        return false;
    }
}

u64 Target::getTitleId() const { return process.getTitleId(); }

} // namespace hydra::debugger
