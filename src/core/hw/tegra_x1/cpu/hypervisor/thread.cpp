#include "core/hw/tegra_x1/cpu/hypervisor/thread.hpp"

#include <mach/mach_time.h>
#include <thread>

#include "core/debugger/debugger_manager.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/cpu.hpp"
#include "core/hw/tegra_x1/cpu/hypervisor/mmu.hpp"

#define CPU (*static_cast<Cpu*>(&CPU_INSTANCE))
#define MMU (*static_cast<Mmu*>(mmu))

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

constexpr u32 PSTATE_SS = (1u << 21);
constexpr u32 MDSCR_EL1_SS = (1u << 0);
constexpr u32 MDSCR_EL1_MDE = (1u << 15);

constexpr u32 DBGBCR_E = (1u << 0);     // Enable bit
constexpr u32 DBGBCR_BAS = (0xfu << 5); // Byte address select

constexpr u64 INTERRUPT_TIME = 16 * 1000 * 1000; // 16ms

enum class ExceptionClass {
    Unknown = 0b000000,
    TrappedWfeWfiWfetWfit = 0b000001,
    TrappedMcrMrcCp15 = 0b000011,
    TrappedMcrrMrrcCp15 = 0b000100,
    TrappedMcrMrcCp14 = 0b000101,
    TrappedLdcStc = 0b000110,
    TrappedSveFpSimd = 0b000111,
    TrappedVmrs = 0b001000,
    TrappedPAuth = 0b001001,
    TrappedLd64bSt64bSt64bvSt64bv0 = 0b001010,
    TrappedMrrcCp14 = 0b001100,
    IllegalExecutionState = 0b001110,
    SvcAarch32 = 0b010001,
    HvcAarch32 = 0b010010,
    SmcAarch32 = 0b010011,
    SvcAarch64 = 0b010101,
    HvcAarch64 = 0b010110,
    SmcAarch64 = 0b010111,
    TrappedMsrMrsSystem = 0b011000,
    TrappedSve = 0b011001,
    TrappedEretEretaaEretab = 0b011010,
    PointerAuthenticationFailure = 0b011100,
    ImplementationDefinedEl3 = 0b011111,
    InstructionAbortLowerEl = 0b100000,
    InstructionAbortSameEl = 0b100001,
    PcAlignmentFault = 0b100010,
    DataAbortLowerEl = 0b100100,
    DataAbortSameEl = 0b100101,
    SpAlignmentFault = 0b100110,
    TrappedFpExceptionAarch32 = 0b101000,
    TrappedFpExceptionAarch64 = 0b101100,
    SErrorInterrupt = 0b101111,
    BreakpointLowerEl = 0b110000,
    BreakpointSameEl = 0b110001,
    SoftwareStepLowerEl = 0b110010,
    SoftwareStepSameEl = 0b110011,
    WatchpointLowerEl = 0b110100,
    WatchpointSameEl = 0b110101,
    BkptAarch32 = 0b111000,
    VectorCatchAarch32 = 0b111010,
    BrkAarch64 = 0b111100,
};

} // namespace

} // namespace hydra::hw::tegra_x1::cpu::hypervisor

ENABLE_ENUM_FORMATTING(
    hydra::hw::tegra_x1::cpu::hypervisor::ExceptionClass, Unknown, "unknown",
    TrappedWfeWfiWfetWfit, "trapped WFE WFI WFET WFIT", TrappedMrrcCp14,
    "trapped MRRC CP14", IllegalExecutionState, "illegal execution state",
    SvcAarch32, "svc AArch32", HvcAarch32, "hvc AArch32", SmcAarch32,
    "smc AArch32", SvcAarch64, "svc AArch64", HvcAarch64, "hvc AArch64",
    SmcAarch64, "smc AArch64", TrappedMsrMrsSystem, "trapped MSR/MRS system",
    TrappedSve, "trapped SVE", TrappedEretEretaaEretab,
    "trapped ERET/ERETAA/ERETAB", PointerAuthenticationFailure,
    "pointer authentication failure", ImplementationDefinedEl3,
    "implementation defined EL3", InstructionAbortLowerEl,
    "instruction abort lower EL", InstructionAbortSameEl,
    "instruction abort same EL", PcAlignmentFault, "PC alignment fault",
    DataAbortLowerEl, "data abort lower EL", DataAbortSameEl,
    "data abort same EL", SpAlignmentFault, "SP alignment fault",
    TrappedFpExceptionAarch32, "trapped FP exception AArch32",
    TrappedFpExceptionAarch64, "trapped FP exception AArch64", SErrorInterrupt,
    "SError interrupt", BreakpointLowerEl, "breakpoint lower EL",
    BreakpointSameEl, "breakpoint same EL", SoftwareStepLowerEl,
    "software step lower EL", SoftwareStepSameEl, "software step same EL",
    WatchpointLowerEl, "watchpoint lower EL", WatchpointSameEl,
    "watchpoint same EL", BkptAarch32, "BKPT AArch32", VectorCatchAarch32,
    "vector catch AArch32", BrkAarch64, "BRK AArch64")

namespace hydra::hw::tegra_x1::cpu::hypervisor {

Thread::Thread(IMmu* mmu, const ThreadCallbacks& callbacks, IMemory* tls_mem,
               vaddr_t tls_mem_base)
    : IThread(mmu, callbacks, tls_mem) {
    // Create
    HV_ASSERT_SUCCESS(hv_vcpu_create(&vcpu, &exit, NULL));

    // TODO: find out what this does
    SetReg(HV_REG_CPSR, 0x3c0);

    SetSysReg(HV_SYS_REG_MAIR_EL1, 0xfful);
    SetSysReg(HV_SYS_REG_TCR_EL1, 0x00000011B5193519ul);
    SetSysReg(HV_SYS_REG_SCTLR_EL1, 0x0000000034D5D925ul);

    // Enable FP and SIMD instructions
    SetSysReg(HV_SYS_REG_CPACR_EL1, 0b11 << 20);

    // Trampoline
    SetSysReg(HV_SYS_REG_VBAR_EL1, KERNEL_REGION_BASE);

    SetSysReg(HV_SYS_REG_TTBR0_EL1, MMU.GetUserPageTable().GetBase());
    SetSysReg(HV_SYS_REG_TTBR1_EL1, CPU.GetKernelPageTable().GetBase());

    // Setup TLS pointer
    SetSysReg(HV_SYS_REG_TPIDRRO_EL0, tls_mem_base);

    // Trap debug access
    HV_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_exceptions(vcpu, true));
    // HYP_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_reg_accesses(vcpu, true));

    // VTimer
    struct mach_timebase_info info;
    mach_timebase_info(&info);

    interrupt_time_delta_ticks =
        ((INTERRUPT_TIME * info.denom) + (info.numer - 1)) / info.numer;

    SetupVTimer();

    // HACK: set LR to loader return address
    // SetReg(HV_REG_LR, 0xffff0000);
}

Thread::~Thread() { hv_vcpu_destroy(vcpu); }

void Thread::Run() {
    // Main run loop
    while (true) {
        ProcessMessages();

        // Run
        DeserializeState();
        HV_ASSERT_SUCCESS(hv_vcpu_run(vcpu));
        exception = (GetReg(HV_REG_PC) >= CPU.GetKernelPageTable().GetBase());
        SerializeState();

        // Handle exit
        if (exit->reason == HV_EXIT_REASON_EXCEPTION) {
            u64 syndrome = exit->exception.syndrome;
            const auto hv_ec =
                static_cast<ExceptionClass>((syndrome >> 26) & 0x3f);

            switch (hv_ec) {
            case ExceptionClass::HvcAarch64: {
                const auto esr =
                    static_cast<u32>(GetSysReg(HV_SYS_REG_ESR_EL1));
                const auto ec = static_cast<ExceptionClass>((esr >> 26) & 0x3f);
                const u64 far = GetSysReg(HV_SYS_REG_FAR_EL1);

                switch (ec) {
                case ExceptionClass::SvcAarch64:
                    callbacks.svc_handler(this, esr & 0xffff);
                    break;
                case ExceptionClass::TrappedMsrMrsSystem: {
                    InstructionTrap(esr);

                    state.pc += 4;
                    break;
                }
                case ExceptionClass::DataAbortLowerEl: {
                    // TODO: use the correct size
                    if (MMU.TrackWrite(Range<vaddr_t>::FromSize(far, 8)))
                        break;

                    bool far_valid = (esr & 0x00000400) == 0;
                    ASSERT_DEBUG(far_valid, Hypervisor, "FAR not valid");

                    if (CONFIG_INSTANCE.GetRecoverFromSegfault()) {
                        LOG_ERROR(
                            Hypervisor,
                            "Data abort (PC: 0x{:08x}, address: 0x{:08x})",
                            state.pc, far);

                        // Just move on to the next instruction
                        state.pc += 4;
                    } else {
                        GET_CURRENT_PROCESS_DEBUGGER().BreakOnThisThread(
                            "Data abort (PC: 0x{:08x}, address: 0x{:08x})",
                            state.pc, far);
                    }
                    break;
                }
                default:
                    LOG_ERROR(
                        Hypervisor,
                        "Unknown HVC code (EC: {}, ESR: 0x{:08x}, PC: "
                        "0x{:08x}, FAR: "
                        "0x{:08x}, VA: 0x{:08x}, PA: 0x{:08x}, instruction: "
                        "0x{:08x})",
                        ec, esr, state.pc, GetSysReg(HV_SYS_REG_FAR_EL1),
                        exit->exception.virtual_address,
                        exit->exception.physical_address,
                        MMU.Read<u32>(state.pc));

                    GET_CURRENT_PROCESS_DEBUGGER().BreakOnThisThread(
                        "unknown HVC code");
                    break;
                }

                // Set the PC to trampoline
                // TODO: most of the time we can skip msr, find out when
                SetReg(HV_REG_PC,
                       KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET);
                break;
            }
            case ExceptionClass::SmcAarch64:
                LOG_FATAL(Hypervisor, "SMC");
                break;
            case ExceptionClass::BrkAarch64:
                GET_CURRENT_PROCESS_DEBUGGER().BreakOnThisThread(
                    "BRK instruction");
                return;
            case ExceptionClass::DataAbortLowerEl: {
                LOG_FATAL(Hypervisor, "This should not happen");
                break;
            }
            case ExceptionClass::BreakpointLowerEl: {
                // Callback
                callbacks.breakpoint_hit();
                break;
            }
            case ExceptionClass::SoftwareStepLowerEl: {
                // HACK: single-stepping in exceptions behaves very weirdly
                if (exception) {
                    state.pstate |= PSTATE_SS;
                    break;
                }

                // Disable SW step
                u64 mdscr_el1 = GetSysReg(HV_SYS_REG_MDSCR_EL1);
                mdscr_el1 &= ~MDSCR_EL1_SS;
                SetSysReg(HV_SYS_REG_MDSCR_EL1, mdscr_el1);

                // Callback
                callbacks.supervisor_pause();
                break;
            }
            default:
                GET_CURRENT_PROCESS_DEBUGGER().BreakOnThisThread(
                    "Unexpected VM exception 0x{:08x} (EC: {}, ESR: "
                    "0x{:08x}, PC: 0x{:08x}, "
                    "VA: "
                    "0x{:08x}, PA: 0x{:08x}, "
                    "instruction: "
                    "0x{:08x})",
                    syndrome, hv_ec, GetSysReg(HV_SYS_REG_ESR_EL1), state.pc,
                    exit->exception.virtual_address,
                    exit->exception.physical_address, MMU.Read<u32>(state.pc));
                break;
            }
        } else if (exit->reason == HV_EXIT_REASON_VTIMER_ACTIVATED) {
            UpdateVTimer();
        } else {
            // TODO: don't cast to u32
            LOG_ERROR(Hypervisor, "Unexpected VM exit reason {}",
                      (u32)exit->reason);

            GET_CURRENT_PROCESS_DEBUGGER().BreakOnThisThread(
                "unexpected VM exit reason");
            break;
        }

        if (callbacks.stop_requested())
            break;
    }
}

void Thread::SetupVTimer() {
    SetSysReg(HV_SYS_REG_CNTV_CTL_EL0, 1);
    SetSysReg(HV_SYS_REG_CNTV_CVAL_EL0,
              mach_absolute_time() + interrupt_time_delta_ticks);
}

void Thread::UpdateVTimer() {
    SetupVTimer();
    hv_vcpu_set_vtimer_mask(vcpu, false);
}

void Thread::SerializeState() {
    for (u8 i = 0; i < 29; i++)
        state.r[i] = GetReg(hv_reg_t(HV_REG_X0 + i));
    state.fp = GetReg(HV_REG_FP);
    state.lr = GetReg(HV_REG_LR);
    state.sp = GetSysReg(HV_SYS_REG_SP_EL0);
    if (exception)
        state.pc = GetSysReg(HV_SYS_REG_ELR_EL1) - 4;
    else
        state.pc = GetReg(HV_REG_PC);
    state.pstate = static_cast<u32>(GetReg(HV_REG_CPSR));
    for (u8 i = 0; i < 32; i++)
        state.v[i] = GetSimdFpReg(i);
    state.fpcr = static_cast<u32>(GetReg(HV_REG_FPCR));
    state.fpsr = static_cast<u32>(GetReg(HV_REG_FPSR));
}

void Thread::DeserializeState() {
    for (u8 i = 0; i < 29; i++)
        SetReg(hv_reg_t(HV_REG_X0 + i), state.r[i]);
    SetReg(HV_REG_FP, state.fp);
    SetReg(HV_REG_LR, state.lr);
    SetSysReg(HV_SYS_REG_SP_EL0, state.sp);
    if (exception)
        SetSysReg(HV_SYS_REG_ELR_EL1, state.pc + 4);
    else
        SetReg(HV_REG_PC, state.pc);
    SetReg(HV_REG_CPSR, state.pstate);
    for (u8 i = 0; i < 32; i++)
        SetSimdFpReg(i, state.v[i]);
    SetReg(HV_REG_FPCR, state.fpcr);
    SetReg(HV_REG_FPSR, state.fpsr);
}

void Thread::InstructionTrap(u32 esr) {
    bool read = (esr & 1) != 0;
    uint rt = (esr >> 5) & 0x1f;

    if (read) {
        // Op0 Op2 Op1 CRn 00000 CRm
        switch ((esr >> 1) & 0x1ffe0f) {
        case 0b11'000'011'1110'00000'0000: // CNTFRQ_EL0
            ONCE(LOG_WARN(Hypervisor, "Frequency"));
            // TODO: correct?
            state.r[rt] = CLOCK_RATE_HZ;
            break;
        case 0b11'001'011'1110'00000'0000:     // CNTPCT_EL0
            state.r[rt] = get_absolute_time(); // TODO: correct?
            break;
        default:
            LOG_FATAL(Hypervisor,
                      "Unhandled system register read (ESR: 0x{:08x})", esr);
            break;
        }
    } else {
        LOG_FATAL(Hypervisor, "Unhandled system register write (ESR: 0x{:08x})",
                  esr);
    }
}

void Thread::ProcessMessages() {
    std::lock_guard<std::mutex> lock(msg_mutex);
    while (!msg_queue.empty()) {
        auto message = msg_queue.front();
        msg_queue.pop();
        switch (message.type) {
        case ThreadMessageType::InsertBreakpoint: {
            const auto addr = message.payload.insert_breakpoint.addr;

            // Enable breakpoints
            // TODO: only do once?
            u64 mdscr_el1 = GetSysReg(HV_SYS_REG_MDSCR_EL1);
            mdscr_el1 |= MDSCR_EL1_MDE;
            SetSysReg(HV_SYS_REG_MDSCR_EL1, mdscr_el1);

            // Find a breakpoint slot
            bool found = false;
            for (u32 slot = 0; slot < MAX_BREAKPOINTS; ++slot) {
                if (breakpoints[slot] == 0x0) {
                    breakpoints[slot] = addr;

                    // DBGBVR
                    SetSysReg(hv_sys_reg_t(HV_SYS_REG_DBGBVR0_EL1 + slot * 8),
                              addr);

                    // DBGBCR
                    SetSysReg(hv_sys_reg_t(HV_SYS_REG_DBGBCR0_EL1 + slot * 8),
                              DBGBCR_E | (0x3 << 1) | DBGBCR_BAS | (0x0 << 20));

                    found = true;
                    break;
                }
            }

            if (!found)
                LOG_ERROR(
                    Hypervisor,
                    "Failed to set breakpoint at address 0x{:08x}: no free "
                    "breakpoint slots",
                    addr);
            break;
        }
        case ThreadMessageType::RemoveBreakpoint: {
            const auto addr = message.payload.remove_breakpoint.addr;

            // Disable breakpoints
            // TODO: disable when no breakpoints are active?
            u64 mdscr_el1 = GetSysReg(HV_SYS_REG_MDSCR_EL1);
            mdscr_el1 &= ~MDSCR_EL1_MDE;
            SetSysReg(HV_SYS_REG_MDSCR_EL1, mdscr_el1);

            // Find the breakpoint slot
            bool found = false;
            for (u32 slot = 0; slot < MAX_BREAKPOINTS; ++slot) {
                if (breakpoints[slot] == addr) {
                    breakpoints[slot] = 0x0;

                    // DBGBVR
                    SetSysReg(hv_sys_reg_t(HV_SYS_REG_DBGBVR0_EL1 + slot * 8),
                              0x0);

                    // DBGBCR
                    SetSysReg(hv_sys_reg_t(HV_SYS_REG_DBGBCR0_EL1 + slot * 8),
                              0x0);

                    found = true;
                    break;
                }
            }

            if (!found)
                LOG_ERROR(
                    Hypervisor,
                    "Failed to remove breakpoint at address 0x{:08x}: no such "
                    "breakpoint",
                    addr);
            break;
        }
        case ThreadMessageType::SingleStep: {
            // Enable SW step
            u64 mdscr_el1 = GetSysReg(HV_SYS_REG_MDSCR_EL1);
            mdscr_el1 |= MDSCR_EL1_SS;
            SetSysReg(HV_SYS_REG_MDSCR_EL1, mdscr_el1);

            // Set PSTATE.SS
            state.pstate |= PSTATE_SS;
            break;
        }
        default:
            LOG_FATAL(Hypervisor, "Unhandled message type");
            break;
        }
    }
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
