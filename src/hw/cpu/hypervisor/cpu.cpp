#include "hw/cpu/hypervisor/cpu.hpp"

#include "common/common.hpp"
#include "hw/mmu/hypervisor/mmu.hpp"
#include "hw/mmu/memory.hpp"
#include <Hypervisor/hv_gic.h>
#include <Hypervisor/hv_vcpu.h>

#define MAX_STACK_TRACE_DEPTH 32

namespace Hydra::HW::CPU::Hypervisor {

CPU::CPU(HW::MMU::Hypervisor::MMU& mmu_) : mmu{mmu_} {
    // Create
    HYP_ASSERT_SUCCESS(hv_vcpu_create(&vcpu, &vcpu_exit, NULL));

    // TODO: what are these?
    SetSysReg(HV_SYS_REG_TCR_EL1, 0x00000011B5193519UL);
    // SetSysReg(HV_SYS_REG_SCTLR_EL1, 0x0000000034D5D925UL);

    // Enable FP and SIMD instructions.
    // TODO: find out how this works
    SetSysReg(HV_SYS_REG_CPACR_EL1, 0b11 << 20);

    SetSysReg(HV_SYS_REG_MAIR_EL1, 0xffUL);

    // Trap debug access
    HYP_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_exceptions(vcpu, true));
    // HYP_ASSERT_SUCCESS(hv_vcpu_set_trap_debug_reg_accesses(vcpu, true));
}

CPU::~CPU() { hv_vcpu_destroy(vcpu); }

void CPU::Run() { HYP_ASSERT_SUCCESS(hv_vcpu_run(vcpu)); }

void CPU::AdvancePC() {
    u64 pc = GetReg(HV_REG_PC);
    SetReg(HV_REG_PC, pc + 4);
}

void CPU::SetupVTimer() {
    SetSysReg(HV_SYS_REG_CNTV_CTL_EL0, 1);
    SetSysReg(HV_SYS_REG_CNTV_CVAL_EL0,
              1000000000000000000); // TODO: set to current time
}

void CPU::UpdateVTimer() {
    SetupVTimer();
    hv_vcpu_set_vtimer_mask(vcpu, false);
}

u64 CPU::GetReg(hv_reg_t reg) const {
    u64 value;
    HYP_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, reg, &value));

    return value;
}

u64 CPU::GetSysReg(hv_sys_reg_t sys_reg) const {
    u64 value;
    HYP_ASSERT_SUCCESS(hv_vcpu_get_sys_reg(vcpu, sys_reg, &value));

    return value;
}

void CPU::SetReg(hv_reg_t reg, u64 value) {
    HYP_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, reg, value));
}

void CPU::SetSysReg(hv_sys_reg_t sys_reg, u64 value) {
    HYP_ASSERT_SUCCESS(hv_vcpu_set_sys_reg(vcpu, sys_reg, value));
}

void CPU::LogRegisters(u32 count) {
    printf("Reg dump:\n");
    for (u32 i = 0; i < count; i++) {
        printf("X%u: 0x%llx\n", i, GetReg((hv_reg_t)(HV_REG_X0 + i)));
    }
}

void CPU::LogStackTrace(HW::MMU::Memory* stack_mem) {
    u64 fp = GetReg(HV_REG_FP);
    u64 lr = GetReg(HV_REG_LR);
    u64 sp = GetSysReg(HV_SYS_REG_SP_EL0);

    printf("Stack trace:\n");
    printf("SP: 0x%08llx\n", sp);

    for (uint64_t frame = 0; fp != 0; frame++) {
        printf("LR = 0x%llx\n", lr - 0x4);
        if (frame == MAX_STACK_TRACE_DEPTH - 1) {
            printf("... (more frames)\n");
            break;
        }

        if (!stack_mem->PtrIsInRange(fp))
            break;

        u64 new_fp = *((u64*)stack_mem->UnmapPtr(fp));
        lr = *((u64*)stack_mem->UnmapPtr(fp + 8));

        fp = new_fp;
    }
}

} // namespace Hydra::HW::CPU::Hypervisor
