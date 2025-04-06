#include "hw/tegra_x1/cpu/hypervisor/thread.hpp"

#include <mach/mach_time.h>

#include "common/functions.hpp"
#include "hw/tegra_x1/cpu/hypervisor/mmu.hpp"

namespace Hydra::HW::TegraX1::CPU::Hypervisor {

constexpr u32 MAX_STACK_TRACE_DEPTH = 32;

constexpr u64 INTERRUPT_TIME = 16 * 1000 * 1000; // 16ms

Thread::Thread(MMU* mmu_, MemoryBase* tls_mem)
    : ThreadBase(tls_mem), mmu{mmu_} {
    // Create
    HV_ASSERT_SUCCESS(hv_vcpu_create(&vcpu, &exit, NULL));

    // TODO: find out what this does
    SetReg(HV_REG_CPSR, 0x3c4);

    SetSysReg(HV_SYS_REG_MAIR_EL1, 0xfful);
    SetSysReg(HV_SYS_REG_TCR_EL1, 0x00000011B5193519ul);
    SetSysReg(HV_SYS_REG_SCTLR_EL1, 0x0000000034D5D925ul);

    // Enable FP and SIMD instructions.
    SetSysReg(HV_SYS_REG_CPACR_EL1, 0b11 << 20);

    // Trap debug access
    HV_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_exceptions(vcpu, true));
    // HYP_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_reg_accesses(vcpu, true));

    // VTimer
    struct mach_timebase_info info;
    auto time = mach_timebase_info(&info);

    interrupt_time_delta_ticks =
        ((INTERRUPT_TIME * info.denom) + (info.numer - 1)) / info.numer;
}

Thread::~Thread() { hv_vcpu_destroy(vcpu); }

void Thread::Configure(const std::function<bool(ThreadBase*, u64)>&
                           svc_handler_,
                       uptr tls_mem_base /*,
  uptr rom_mem_base*/, uptr stack_mem_end) {
    svc_handler = svc_handler_;

    // Trampoline
    SetSysReg(HV_SYS_REG_VBAR_EL1, KERNEL_REGION_BASE);

    SetSysReg(HV_SYS_REG_TTBR0_EL1, mmu->GetUserPageTable().GetBase());
    SetSysReg(HV_SYS_REG_TTBR1_EL1, mmu->GetKernelPageTable().GetBase());

    // Initialize the stack pointer
    SetSysReg(HV_SYS_REG_SP_EL0, stack_mem_end);
    // TODO: set SP_EL1 as well?

    // Setup TLS pointer
    // TODO: offset by thread id * some alignment?
    SetSysReg(HV_SYS_REG_TPIDRRO_EL0, tls_mem_base);
}

void Thread::Run() {
    // Main run loop
    bool running = true;
    while (running) {
        HV_ASSERT_SUCCESS(hv_vcpu_run(vcpu));

        if (exit->reason == HV_EXIT_REASON_EXCEPTION) {
            u64 syndrome = exit->exception.syndrome;
            u8 hvEc = (syndrome >> 26) & 0x3f;
            u64 pc = GetReg(HV_REG_PC);

            if (hvEc == 0x16) { // HVC
                u64 esr = GetSysReg(HV_SYS_REG_ESR_EL1);
                u8 ec = (esr >> 26) & 0x3f;

                u64 elr = GetSysReg(HV_SYS_REG_ELR_EL1);

                u64 far = GetSysReg(HV_SYS_REG_FAR_EL1);

                // u64 spsr = cpu->GetSysReg(HV_SYS_REG_SPSR_EL1);
                // u64 mode = (spsr >> 2) & 0x3;

                u32 instruction = mmu->Load<u32>(elr);

                switch (ec) {
                case 0x15:
                    // Debug
                    LogStackTrace(elr);
                    // cpu->LogRegisters();

                    running = svc_handler(this, esr & 0xffff);

                    break;
                case 0x18:
                    LOG_DEBUG(Hypervisor, "MSR MSR");
                    // TODO: implement
                    throw;
                    break;
                case 0x25: {
                    // Debug
                    // cpu->LogStackTrace(horizon.GetKernel().GetStackMemory(),
                    //                   elr);

                    // LOG_DEBUG(Hypervisor,
                    //           "Data abort (PC: 0x{:08x}, FAR: 0x{:08x}, "
                    //           "instruction: 0x{:08x})",
                    //           elr, far, instruction);

                    bool far_valid = (esr & 0x00000400) == 0;
                    ASSERT(far_valid, Hypervisor, "FAR not valid");

                    DataAbort(instruction, far, elr);

                    break;
                }
                default:
                    // Debug
                    LogStackTrace(elr);

                    LOG_ERROR(
                        Hypervisor,
                        "Unknown HVC code (EC: 0x{:08x}, ESR: 0x{:08x}, PC: "
                        "0x{:08x}, FAR: "
                        "0x{:08x}, VA: 0x{:08x}, PA: 0x{:08x})",
                        ec, esr, GetSysReg(HV_SYS_REG_ELR_EL1),
                        GetSysReg(HV_SYS_REG_FAR_EL1),
                        exit->exception.virtual_address,
                        exit->exception.physical_address);

                    break;
                }

                // Set the PC to trampoline
                // TODO: most of the time we can skip msr, find out when
                SetReg(HV_REG_PC,
                       KERNEL_REGION_BASE + EXCEPTION_TRAMPOLINE_OFFSET);
            } else if (hvEc == 0x17) { // SMC
                LOG_WARNING(Hypervisor, "SMC instruction");

                AdvancePC();
            } else if (hvEc == 0x18) {
                // TODO: this should not happen

                // Debug
                LogStackTrace(pc);

                LOG_DEBUG(Hypervisor, "MSR MRS instruction");

                // Manually execute the instruction
                u32 instruction = mmu->Load<u32>(pc);

                u8 opcode = extract_bits<u8, 24, 8>(instruction);
                u8 rt = extract_bits<u8, 0, 5>(instruction);

                u8 op0 = extract_bits<u8, 20, 2>(instruction);
                u8 op1 = extract_bits<u8, 16, 3>(instruction);
                u8 crn = extract_bits<u8, 12, 4>(instruction);
                u8 crm = extract_bits<u8, 8, 4>(instruction);
                u8 op2 = extract_bits<u8, 5, 3>(instruction);

                u64 value = 0;
                if (op0 == 3 && op1 == 3 && crn == 14 && crm == 0 &&
                    op2 == 1) { // cntpct_el0
                    value = mach_absolute_time();
                } else {
                    LOG_ERROR(Hypervisor,
                              "Unknown MSR instruction (opcode: 0x{:08x}, "
                              "rt: {}, op0: {}, op1: {}, crn: {}, crm: {}, "
                              "op2: {})",
                              opcode, rt, op0, op1, crn, crm, op2);
                }

                SetReg((hv_reg_t)(HV_REG_X0 + rt), value);

                // Set the return address
                // TODO: correct?
                // u64 elr = cpu->GetSysReg(HV_SYS_REG_ELR_EL1);
                // cpu->SetSysReg(HV_SYS_REG_ELR_EL1, elr + 4);
                AdvancePC();
            } else if (hvEc == 0x3C) { // BRK
                LogRegisters(true);

                LOG_ERROR(Hypervisor, "BRK instruction");

                break;
            } else {
                // Debug
                LogStackTrace(pc);
                LogRegisters();

                LOG_ERROR(
                    Hypervisor,
                    "Unexpected VM exception 0x{:08x} (EC: 0x{:08x}, ESR: "
                    "0x{:08x}, PC: 0x{:08x}, "
                    "VA: "
                    "0x{:08x}, PA: 0x{:08x}, ELR: 0x{:08x}, "
                    "instruction: "
                    "0x{:08x})",
                    syndrome, hvEc, GetSysReg(HV_SYS_REG_ESR_EL1), pc,
                    GetSysReg(HV_SYS_REG_ELR_EL1),
                    exit->exception.virtual_address,
                    exit->exception.physical_address, mmu->Load<u32>(pc));

                break;
            }
        } else if (exit->reason == HV_EXIT_REASON_VTIMER_ACTIVATED) {
            UpdateVTimer();
        } else {
            // TODO: don't cast to u32
            LOG_ERROR(Hypervisor, "Unexpected VM exit reason {}",
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
    LOG_DEBUG(CPU, "Reg dump:");
    for (u32 i = 0; i < count; i++) {
        LOG_DEBUG(CPU, "X{}: 0x{:08x}", i, GetRegX(i));
    }
    if (simd) {
        for (u32 i = 0; i < count; i++) {
            auto reg = GetRegQ(i);
            LOG_DEBUG(CPU, "Q{}: 0x{:08x}{:08x}", i, *(u64*)&reg,
                      *((u64*)&reg + 1)); // TODO: correct?
        }
    }
    LOG_DEBUG(CPU, "SP: 0x{:08x}", GetSysReg(HV_SYS_REG_SP_EL0));
}

void Thread::LogStackTrace(uptr pc) {
    u64 fp = GetReg(HV_REG_FP);
    u64 lr = GetReg(HV_REG_LR);
    u64 sp = GetSysReg(HV_SYS_REG_SP_EL0);

    LOG_DEBUG(CPU, "Stack trace:");
    // LOG_DEBUG(CPU, "SP: 0x{:08x}", sp);
    LOG_DEBUG(CPU, "0x{:08x}", pc);

    for (uint64_t frame = 0; fp != 0; frame++) {
        LOG_DEBUG(CPU, "0x{:08x}", lr - 0x4);
        if (frame == MAX_STACK_TRACE_DEPTH - 1) {
            LOG_DEBUG(CPU, "... (more frames)");
            break;
        }

        // if (!stack_mem->AddrIsInRange(fp))
        //     break;
        // HACK
        if (fp < 0x10000000 || fp >= 0x20000000) {
            LOG_WARNING(Hypervisor, "Currputed stack");
            break;
        }

        u64 new_fp = mmu->Load<u64>(fp);
        lr = mmu->Load<u64>(fp + 8);

        fp = new_fp;
    }
}

void Thread::DataAbort(u32 instruction, u64 far, u64 elr) {
    LOG_WARNING(Hypervisor,
                "PC: 0x{:08x}, instruction: 0x{:08x}, FAR: 0x{:08x} ", elr,
                instruction, far);

    // Set the return address
    // TODO: correct?
    SetSysReg(HV_SYS_REG_ELR_EL1, elr + 4);
}

} // namespace Hydra::HW::TegraX1::CPU::Hypervisor
