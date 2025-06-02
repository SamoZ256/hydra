#include "core/hw/tegra_x1/cpu/hypervisor/thread.hpp"

#include <mach/mach_time.h>
#include <thread>

#include "core/hw/tegra_x1/cpu/hypervisor/mmu.hpp"

namespace hydra::hw::tegra_x1::cpu::hypervisor {

namespace {

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

Thread::~Thread() { hv_vcpu_destroy(vcpu); }

void Thread::Initialize(const std::function<bool(ThreadBase*, u64)>&
                            svc_handler_,
                        uptr tls_mem_base /*,
   uptr rom_mem_base*/, uptr stack_mem_end) {
    svc_handler = svc_handler_;

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

    SetSysReg(HV_SYS_REG_TTBR0_EL1, mmu->GetUserPageTable().GetBase());
    SetSysReg(HV_SYS_REG_TTBR1_EL1, mmu->GetKernelPageTable().GetBase());

    // Initialize the stack pointer
    SetSysReg(HV_SYS_REG_SP_EL0, stack_mem_end);
    // TODO: set SP_EL1 as well?

    // Setup TLS pointer
    SetSysReg(HV_SYS_REG_TPIDRRO_EL0, tls_mem_base);

    // Trap debug access
    HV_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_exceptions(vcpu, true));
    // HYP_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_reg_accesses(vcpu, true));

    // VTimer
    struct mach_timebase_info info;
    auto time = mach_timebase_info(&info);

    interrupt_time_delta_ticks =
        ((INTERRUPT_TIME * info.denom) + (info.numer - 1)) / info.numer;

    SetupVTimer();
}

void Thread::Run() {
    // Main run loop
    bool running = true;
    while (running) {
        HV_ASSERT_SUCCESS(hv_vcpu_run(vcpu));

        if (exit->reason == HV_EXIT_REASON_EXCEPTION) {
            LogStackTrace();

            u64 syndrome = exit->exception.syndrome;
            const auto hv_ec =
                static_cast<ExceptionClass>((syndrome >> 26) & 0x3f);
            u64 pc = GetReg(HV_REG_PC);

            switch (hv_ec) {
            case ExceptionClass::HvcAarch64: {
                u64 esr = GetSysReg(HV_SYS_REG_ESR_EL1);
                const auto ec = static_cast<ExceptionClass>((esr >> 26) & 0x3f);

                u64 elr = GetSysReg(HV_SYS_REG_ELR_EL1);

                u64 far = GetSysReg(HV_SYS_REG_FAR_EL1);

                u32 instruction = mmu->Load<u32>(elr);

                switch (ec) {
                case ExceptionClass::SvcAarch64:
                    running = svc_handler(this, esr & 0xffff);
                    break;
                case ExceptionClass::TrappedMsrMrsSystem: {
                    InstructionTrap(esr);

                    u64 elr = GetSysReg(HV_SYS_REG_ELR_EL1);
                    SetSysReg(HV_SYS_REG_ELR_EL1, elr + 4);
                    break;
                }
                case ExceptionClass::DataAbortLowerEl: {
                    bool far_valid = (esr & 0x00000400) == 0;
                    ASSERT_DEBUG(far_valid, Hypervisor, "FAR not valid");

                    DataAbort(instruction, far, elr);
                    break;
                }
                default:
                    LOG_FATAL(
                        Hypervisor,
                        "Unknown HVC code (EC: {}, ESR: 0x{:08x}, PC: "
                        "0x{:08x}, FAR: "
                        "0x{:08x}, VA: 0x{:08x}, PA: 0x{:08x}, instruction: "
                        "0x{:08x})",
                        ec, esr, GetSysReg(HV_SYS_REG_ELR_EL1),
                        GetSysReg(HV_SYS_REG_FAR_EL1),
                        exit->exception.virtual_address,
                        exit->exception.physical_address, instruction);
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
                LogRegisters(true);

                LOG_FATAL(Hypervisor, "BRK instruction");
                running = false;
                break;
            case ExceptionClass::DataAbortLowerEl:
                LOG_ERROR(Hypervisor, "This should not happen");
                AdvancePC();
                break;
            default:
                // Debug
                LogRegisters();

                LOG_FATAL(Hypervisor,
                          "Unexpected VM exception 0x{:08x} (EC: {}, ESR: "
                          "0x{:08x}, PC: 0x{:08x}, "
                          "VA: "
                          "0x{:08x}, PA: 0x{:08x}, ELR: 0x{:08x}, "
                          "instruction: "
                          "0x{:08x})",
                          syndrome, hv_ec, GetSysReg(HV_SYS_REG_ESR_EL1), pc,
                          GetSysReg(HV_SYS_REG_ELR_EL1),
                          exit->exception.virtual_address,
                          exit->exception.physical_address, mmu->Load<u32>(pc));
                break;
            }
        } else if (exit->reason == HV_EXIT_REASON_VTIMER_ACTIVATED) {
            UpdateVTimer();
        } else {
            // TODO: don't cast to u32
            LOG_FATAL(Hypervisor, "Unexpected VM exit reason {}",
                      (u32)exit->reason);
            break;
        }
    }
}

void Thread::AdvancePC() {
    u64 pc = GetReg(HV_REG_PC);
    SetReg(HV_REG_PC, pc + 4);
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

void Thread::LogRegisters(bool simd, u32 count) {
    LOG_DEBUG(Hypervisor, "Reg dump:");
    for (u32 i = 0; i < count; i++) {
        LOG_DEBUG(Hypervisor, "X{}: 0x{:08x}", i, GetRegX(i));
    }
    if (simd) {
        for (u32 i = 0; i < count; i++) {
            auto reg = GetRegQ(i);
            LOG_DEBUG(Hypervisor, "Q{}: 0x{:08x}{:08x}", i, *(u64*)&reg,
                      *((u64*)&reg + 1)); // TODO: correct?
        }
    }
    LOG_DEBUG(Hypervisor, "SP: 0x{:08x}", GetSysReg(HV_SYS_REG_SP_EL0));
}

void Thread::LogStackTraceImpl() {
    u64 fp = GetReg(HV_REG_FP);
    u64 lr = GetReg(HV_REG_LR);
    u64 sp = GetSysReg(HV_SYS_REG_SP_EL0);

    LOG_DEBUG(Hypervisor, "Stack trace:");
    LOG_DEBUG(Hypervisor, "0x{:08x}", GetReg(HV_REG_PC));
    LOG_DEBUG(Hypervisor, "0x{:08x}", GetSysReg(HV_SYS_REG_ELR_EL1));

    for (uint64_t frame = 0; fp != 0; frame++) {
        LOG_DEBUG(Hypervisor, "0x{:08x}", lr - 0x4);
        if (frame == MAX_STACK_TRACE_DEPTH - 1) {
            LOG_DEBUG(Hypervisor, "... (more frames)");
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

void Thread::InstructionTrap(u32 esr) {
    bool read = (esr & 1) != 0;
    uint rt = (esr >> 5) & 0x1f;

    if (read) {
        // Op0 Op2 Op1 CRn 00000 CRm
        switch ((esr >> 1) & 0x1ffe0f) {
        case 0b11'000'011'1110'00000'0000: // CNTFRQ_EL0
            ONCE(LOG_WARN(Hypervisor, "Frequency"));
            // TODO: correct?
            SetRegX(rt, CLOCK_RATE_HZ);
            break;
        case 0b11'001'011'1110'00000'0000:    // CNTPCT_EL0
            SetRegX(rt, get_absolute_time()); // TODO: correct?
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

void Thread::DataAbort(u32 instruction, u64 far, u64 elr) {
    ONCE(LOG_WARN(Hypervisor,
                  "PC: 0x{:08x}, instruction: 0x{:08x}, FAR: 0x{:08x}", elr,
                  instruction, far));

    // Set the return address
    // TODO: correct?
    SetSysReg(HV_SYS_REG_ELR_EL1, elr + 4);
}

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
