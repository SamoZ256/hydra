#include "core/hw/tegra_x1/cpu/dynarmic/thread.hpp"

#include <dynarmic/interface/exclusive_monitor.h>
#include <mach/mach_time.h>

#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"
#include "dynarmic/interface/optimization_flags.h"

#define MMU static_cast<Mmu*>(mmu)

namespace hydra::hw::tegra_x1::cpu::dynarmic {

// HACK
static Dynarmic::ExclusiveMonitor
    g_exclusive_monitor(4); // TODO: don't hardcode core count

Thread::Thread(IMmu* mmu, const svc_handler_fn_t& svc_handler,
               const stop_requested_fn_t& stop_requested, IMemory* tls_mem,
               vaddr_t tls_mem_base, vaddr_t stack_mem_end)
    : IThread(mmu, svc_handler, stop_requested, tls_mem) {
    tpidrro_el0 = tls_mem_base;

    // Create JIT
    Dynarmic::A64::UserConfig config{};
    config.callbacks = this;

    // Multi-process state
    config.processor_id = 0; // TODO: don't hardcode this
    config.global_monitor = &g_exclusive_monitor;

    // System registers
    config.tpidrro_el0 = &tpidrro_el0;
    config.tpidr_el0 = nullptr; // TODO: what is this?
    config.dczid_el0 = 4;
    config.ctr_el0 = 0x8444c004;
    config.cntfrq_el0 = CLOCK_RATE_HZ;

    // Unpredictable instructions
    config.define_unpredictable_behaviour = true;

    // Timing
    // TODO: correct?
    config.wall_clock_cntpct = true;
    config.enable_cycle_counting = false;

    // Code cache size
    config.code_cache_size = 128 * 1024 * 1024; // 128_MiB;

    // TODO: make this configurable
    // config.optimizations = Dyn::no_optimizations;

    jit = new Dynarmic::A64::Jit(config);

    jit->SetSP(stack_mem_end);
}

Thread::~Thread() { delete jit; }

void Thread::Run() { jit->Run(); }

void Thread::LogRegisters(bool simd, u32 count) {
    // TODO: implement
    LOG_FUNC_NOT_IMPLEMENTED(Dynarmic);
}

u8 Thread::MemoryRead8(u64 addr) { return MMU->Load<u8>(addr); }

u16 Thread::MemoryRead16(u64 addr) { return MMU->Load<u16>(addr); }

u32 Thread::MemoryRead32(u64 addr) { return MMU->Load<u32>(addr); }

u64 Thread::MemoryRead64(u64 addr) { return MMU->Load<u64>(addr); }

Dynarmic::A64::Vector Thread::MemoryRead128(u64 addr) {
    return MMU->Load<Dynarmic::A64::Vector>(addr);
}

std::optional<u32> Thread::MemoryReadCode(u64 addr) {
    return MMU->Load<u32>(addr);
}

void Thread::MemoryWrite8(u64 addr, u8 value) { MMU->Store(addr, value); }

void Thread::MemoryWrite16(u64 addr, u16 value) { MMU->Store(addr, value); }

void Thread::MemoryWrite32(u64 addr, u32 value) { MMU->Store(addr, value); }

void Thread::MemoryWrite64(u64 addr, u64 value) { MMU->Store(addr, value); }

void Thread::MemoryWrite128(u64 addr, Dynarmic::A64::Vector value) {
    MMU->Store(addr, value);
}

bool Thread::MemoryWriteExclusive8(u64 addr, u8 value, u8) {
    MMU->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive16(u64 addr, u16 value, u16) {
    MMU->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive32(u64 addr, u32 value, u32) {
    MMU->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive64(u64 addr, u64 value, u64 expected) {
    MMU->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive128(u64 addr, Dynarmic::A64::Vector value,
                                     Dynarmic::A64::Vector expected) {
    MMU->StoreExclusive(addr, value);
    return true;
}

void Thread::CallSVC(u32 svc) {
    svc_handler(this, svc);
    CheckForStopRequest();
}

void Thread::ExceptionRaised(u64 pc, Dynarmic::A64::Exception exception) {
    // TODO: handle the exception
    // TODO: debugger break
    LOG_FATAL(Dynarmic, "Exception");
}

u64 Thread::GetCNTPCT() { return get_absolute_time(); }

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
