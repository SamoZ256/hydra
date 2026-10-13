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
constexpr u64 WATCHPOINT_MAX_SIZE = 8;
constexpr vaddr_t WATCHPOINT_OFFSET_MASK = 0x7;
constexpr i64 STOP_POLL_INTERVAL_NS = 1000ll * 1000ll;

} // namespace

using hw::tegra_x1::cpu::HaltReason;
using hw::tegra_x1::cpu::Watchpoint;

Target::Target(horizon::kernel::Process& process_)
    : process{process_},
      breakpoint_slots{
          process_.getSystem().getCpu().getFeatures().breakpoint_slots},
      watchpoint_slots{
          process_.getSystem().getCpu().getFeatures().watchpoint_slots},
      native_breakpoints{breakpoint_slots != 0} {}

thread_id_t Target::registerThread(horizon::kernel::GuestThread* guest_thread) {
    std::scoped_lock lock(mutex);

    const auto id = next_thread_id++;
    auto* cpu_thread = guest_thread->getThread();
    threads.emplace(id, ThreadEntry{.guest_thread = guest_thread,
                                    .cpu_thread = cpu_thread,
                                    .resumed = !stopping});

    if (native_breakpoints) {
        for (const auto& [addr, _] : breakpoints)
            cpu_thread->insertBreakpoint(addr);
    }
    for (const auto& watchpoint : watchpoints)
        cpu_thread->insertWatchpoint(watchpoint);

    if (stopping &&
        process.getState() == horizon::kernel::ProcessState::DebugSuspended) {
        guest_thread->supervisorPause();
        cpu_thread->requestHalt();
    } else if (partial_resume) {
        guest_thread->supervisorResume();
    }

    return id;
}

void Target::unregisterThread(thread_id_t id) {
    std::scoped_lock lock(mutex);

    const auto it = threads.find(id);
    if (it == threads.end())
        return;

    const bool was_resumed = it->second.resumed;
    threads.erase(it);

    if (was_resumed && !stopping &&
        std::ranges::none_of(
            threads, [](const auto& pair) { return pair.second.resumed; }))
        stopping = true;
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

std::optional<StopInfo> Target::getStopInfo(thread_id_t id) const {
    std::scoped_lock lock(mutex);

    const auto* entry = findEntry(id);
    if (entry == nullptr)
        return std::nullopt;

    return entry->stop;
}

std::vector<ThreadSnapshot> Target::getThreadSnapshots() const {
    std::scoped_lock lock(mutex);

    std::vector<ThreadSnapshot> snapshots;
    snapshots.reserve(threads.size());
    for (const auto& [id, entry] : threads)
        snapshots.push_back(
            {.id = id, .stop = entry.stop, .resumed = entry.resumed});

    return snapshots;
}

void Target::pauseAll() {
    std::scoped_lock run_lock(run_control_mutex);
    if (!isProcessRunning())
        return;

    {
        std::scoped_lock lock(mutex);
        stopping = true;
        partial_resume = false;
    }

    process.supervisorPause();
    kickRunningThreads();
}

void Target::resumeAll() {
    std::vector<thread_id_t> resumed;
    {
        std::scoped_lock run_lock(run_control_mutex);
        if (!isProcessRunning())
            return;

        {
            std::scoped_lock lock(mutex);
            for (auto& [id, entry] : threads) {
                if (entry.stop)
                    resumed.push_back(id);
                entry.stop = std::nullopt;
                entry.stepping = false;
                entry.resumed = true;
            }
            stopping = false;
            partial_resume = false;
        }

        process.supervisorResume();
    }

    if (halt_state_observer) {
        for (const auto id : resumed)
            halt_state_observer(id, std::nullopt);
    }
}

bool Target::resume(std::span<const ResumeAction> actions) {
    std::scoped_lock run_lock(run_control_mutex);
    if (!isProcessRunning())
        return false;

    std::vector<thread_id_t> resumed;
    bool all;
    {
        std::scoped_lock lock(mutex);

        std::vector<std::pair<ThreadEntry*, ResumeKind>> entries;
        for (const auto& action : actions) {
            auto* entry = findEntry(action.id);
            if (entry == nullptr ||
                std::ranges::any_of(entries, [entry](const auto& pair) {
                    return pair.first == entry;
                }))
                continue;

            entries.emplace_back(entry, action.kind);
            resumed.push_back(action.id);
        }

        if (entries.empty())
            return false;

        for (auto& [_, entry] : threads)
            entry.resumed = false;

        for (auto& [entry, kind] : entries) {
            entry->stop = std::nullopt;
            entry->stepping = kind == ResumeKind::Step;
            entry->resumed = true;
            if (entry->stepping)
                entry->cpu_thread->singleStep();
        }

        all = entries.size() == threads.size();
        stopping = false;
        partial_resume = !all;

        if (!all) {
            for (auto& [entry, _] : entries)
                entry->guest_thread->supervisorResume();
        }
    }

    if (all)
        process.supervisorResume();

    if (halt_state_observer) {
        for (const auto id : resumed)
            halt_state_observer(id, std::nullopt);
    }

    return true;
}

void Target::stop() { process.stop(); }

bool Target::isStopping() const {
    std::scoped_lock lock(mutex);
    return stopping;
}

bool Target::isAllStopped() const {
    std::scoped_lock lock(mutex);
    return std::ranges::all_of(threads, [this](const auto& pair) {
        return isStateAccessible(pair.second);
    });
}

void Target::notifyHalted(horizon::kernel::GuestThread* guest_thread,
                          const hw::tegra_x1::cpu::HaltInfo& halt) {
    auto reason = halt.reason;
    auto id = invalid<thread_id_t>();
    std::optional<StopInfo> stop;
    std::scoped_lock run_lock(run_control_mutex);
    {
        std::scoped_lock lock(mutex);

        ThreadEntry* entry = nullptr;
        for (auto& [entry_id, candidate] : threads) {
            if (candidate.guest_thread == guest_thread) {
                id = entry_id;
                entry = &candidate;
                break;
            }
        }

        if (entry == nullptr)
            return;

        const auto pc = entry->cpu_thread->getState().pc;
        if (reason == HaltReason::StepComplete && !entry->stepping)
            return;

        if (isStaleTrap(reason, pc)) {
            if (!entry->stepping)
                return;

            reason = HaltReason::StepComplete;
        }

        if (reason == HaltReason::BrkInstruction && breakpoints.contains(pc))
            reason = HaltReason::Breakpoint;

        std::optional<Watchpoint> watchpoint;
        if (reason == HaltReason::Watchpoint)
            watchpoint = findWatchpoint(halt.access_addr);

        entry->stop = StopInfo{.reason = reason,
                               .sequence = ++stop_sequence,
                               .watchpoint = watchpoint};
        entry->stepping = false;
        stopping = true;
        partial_resume = false;
        stop = entry->stop;
    }

    if (isProcessRunning())
        process.supervisorPause();
    kickRunningThreads();

    if (halt_state_observer)
        halt_state_observer(id, stop);
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
        if (addr == 0 || breakpoints.size() >= breakpoint_slots)
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

std::vector<vaddr_t> Target::getBreakpoints() const {
    std::scoped_lock lock(mutex);

    std::vector<vaddr_t> addrs;
    addrs.reserve(breakpoints.size());
    for (const auto& [addr, _] : breakpoints)
        addrs.push_back(addr);

    return addrs;
}

bool Target::insertWatchpoint(const Watchpoint& watchpoint) {
    std::scoped_lock lock(mutex);

    if (watchpoint_slots == 0 || watchpoint.size == 0 ||
        watchpoint.size > WATCHPOINT_MAX_SIZE ||
        (watchpoint.addr & WATCHPOINT_OFFSET_MASK) + watchpoint.size >
            WATCHPOINT_MAX_SIZE)
        return false;

    if (watchpoints.contains(watchpoint))
        return true;

    if (watchpoints.size() >= watchpoint_slots)
        return false;

    watchpoints.insert(watchpoint);
    for (const auto& [_, entry] : threads)
        entry.cpu_thread->insertWatchpoint(watchpoint);

    return true;
}

bool Target::removeWatchpoint(const Watchpoint& watchpoint) {
    std::scoped_lock lock(mutex);

    if (watchpoints.erase(watchpoint) == 0)
        return false;

    for (const auto& [_, entry] : threads)
        entry.cpu_thread->removeWatchpoint(watchpoint);

    return true;
}

void Target::removeAllWatchpoints() {
    std::scoped_lock lock(mutex);

    for (const auto& watchpoint : watchpoints) {
        for (const auto& [_, entry] : threads)
            entry.cpu_thread->removeWatchpoint(watchpoint);
    }
    watchpoints.clear();
}

Target::ThreadEntry* Target::findEntry(thread_id_t id) {
    const auto it = threads.find(id);
    return it == threads.end() ? nullptr : &it->second;
}

const Target::ThreadEntry* Target::findEntry(thread_id_t id) const {
    const auto it = threads.find(id);
    return it == threads.end() ? nullptr : &it->second;
}

bool Target::isStaleTrap(HaltReason reason, vaddr_t pc) const {
    if (breakpoints.contains(pc))
        return false;

    switch (reason) {
    case HaltReason::Breakpoint:
        return true;
    case HaltReason::Watchpoint:
        return watchpoints.empty();
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

void Target::kickRunningThreads() {
    std::scoped_lock lock(mutex);
    if (!stopping)
        return;

    for (const auto& [_, entry] : threads) {
        if (entry.guest_thread == horizon::kernel::tls_current_thread)
            continue;

        if (!entry.guest_thread->isSupervisorPaused())
            entry.cpu_thread->requestHalt();
    }
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

std::optional<Watchpoint> Target::findWatchpoint(vaddr_t access_addr) const {
    if (watchpoints.empty())
        return std::nullopt;

    for (const auto& watchpoint : watchpoints) {
        if (access_addr >= watchpoint.addr &&
            access_addr < watchpoint.addr + watchpoint.size)
            return watchpoint;
    }

    for (const auto& watchpoint : watchpoints) {
        if ((access_addr & ~WATCHPOINT_OFFSET_MASK) ==
            (watchpoint.addr & ~WATCHPOINT_OFFSET_MASK))
            return watchpoint;
    }

    for (const auto& watchpoint : watchpoints) {
        if (watchpoint.addr + watchpoint.size > access_addr)
            return watchpoint;
    }

    return *std::ranges::min_element(
        watchpoints, [access_addr](const auto& a, const auto& b) {
            const auto da = a.addr > access_addr ? a.addr - access_addr
                                                 : access_addr - a.addr;
            const auto db = b.addr > access_addr ? b.addr - access_addr
                                                 : access_addr - b.addr;
            return da < db;
        });
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
