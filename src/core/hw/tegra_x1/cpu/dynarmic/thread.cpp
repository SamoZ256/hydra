#include "core/hw/tegra_x1/cpu/dynarmic/thread.hpp"

#include <dynarmic/interface/exclusive_monitor.h>
#include <mach/mach_time.h>

#include "core/hw/tegra_x1/cpu/dynarmic/mmu.hpp"
#include "dynarmic/interface/optimization_flags.h"

namespace hydra::hw::tegra_x1::cpu::dynarmic {

// HACK
static Dynarmic::ExclusiveMonitor
    g_exclusive_monitor(4); // TODO: don't hardcode core count

Thread::~Thread() { delete jit; }

void Thread::Initialize(
    const std::function<bool(ThreadBase*, u64)>& svc_handler_,
    uptr tls_mem_base, uptr stack_mem_end) {
    svc_handler = svc_handler_;
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

void Thread::Run() { jit->Run(); }

void Thread::LogRegisters(bool simd, u32 count) {
    // TODO: implement
    LOG_FUNC_NOT_IMPLEMENTED(Dynarmic);
}

void Thread::LogStackTraceImpl() {
    u64 fp = jit->GetRegister(29);
    u64 lr = jit->GetRegister(30);
    u64 sp = jit->GetSP();

    LOG_DEBUG(CPU, "Stack trace:");
    LOG_DEBUG(CPU, "0x{:08x}", jit->GetPC());
    // TODO: ELR?

    for (uint64_t frame = 0; fp != 0; frame++) {
        LOG_DEBUG(CPU, "0x{:08x}", lr - 0x4);
        if (frame == MAX_STACK_TRACE_DEPTH - 1) {
            LOG_DEBUG(CPU, "... (more frames)");
            break;
        }

        // HACK
        // if (fp < 0x10000000 || fp >= 0x20000000) {
        //    LOG_WARN(Hypervisor, "Currputed stack");
        //    break;
        //}

        u64 new_fp = mmu->Load<u64>(fp);
        lr = mmu->Load<u64>(fp + 8);

        fp = new_fp;
    }
}

u8 Thread::MemoryRead8(u64 addr) { return mmu->Load<u8>(addr); }

u16 Thread::MemoryRead16(u64 addr) { return mmu->Load<u16>(addr); }

u32 Thread::MemoryRead32(u64 addr) { return mmu->Load<u32>(addr); }

u64 Thread::MemoryRead64(u64 addr) { return mmu->Load<u64>(addr); }

Dynarmic::A64::Vector Thread::MemoryRead128(u64 addr) {
    return mmu->Load<Dynarmic::A64::Vector>(addr);
}

std::optional<u32> Thread::MemoryReadCode(u64 addr) {
    return mmu->Load<u32>(addr);
}

void Thread::MemoryWrite8(u64 addr, u8 value) { mmu->Store(addr, value); }

void Thread::MemoryWrite16(u64 addr, u16 value) { mmu->Store(addr, value); }

void Thread::MemoryWrite32(u64 addr, u32 value) { mmu->Store(addr, value); }

void Thread::MemoryWrite64(u64 addr, u64 value) { mmu->Store(addr, value); }

void Thread::MemoryWrite128(u64 addr, Dynarmic::A64::Vector value) {
    mmu->Store(addr, value);
}

bool Thread::MemoryWriteExclusive8(u64 addr, u8 value, u8) {
    mmu->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive16(u64 addr, u16 value, u16) {
    mmu->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive32(u64 addr, u32 value, u32) {
    mmu->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive64(u64 addr, u64 value, u64 expected) {
    mmu->StoreExclusive(addr, value);
    return true;
}

bool Thread::MemoryWriteExclusive128(u64 addr, Dynarmic::A64::Vector value,
                                     Dynarmic::A64::Vector expected) {
    mmu->StoreExclusive(addr, value);
    return true;
}

void Thread::CallSVC(u32 svc) {
    LogStackTrace();
    bool running = svc_handler(this, svc);
    if (!running)
        jit->HaltExecution();
}

void Thread::ExceptionRaised(u64 pc, Dynarmic::A64::Exception exception) {
    LogStackTrace();

    // TODO: handle the exception
    LOG_FATAL(Dynarmic, "Exception");
}

u64 Thread::GetCNTPCT() { return get_absolute_time(); }

} // namespace hydra::hw::tegra_x1::cpu::dynarmic
