#pragma once

#include "core/hw/tegra_x1/cpu/hypervisor/const.hpp"
#include "core/hw/tegra_x1/cpu/thread.hpp"

namespace hydra::horizon {
class OS;
}

namespace hydra::hw::tegra_x1::cpu {
class IMemory;
}

namespace hydra::hw::tegra_x1::cpu::hypervisor {

class Cpu;
class Mmu;

class Thread : public IThread {
  public:
    Thread(IMmu* mmu, const svc_handler_fn_t& svc_handler,
           const stop_requested_fn_t& stop_requested, IMemory* tls_mem,
           vaddr_t tls_mem_base, vaddr_t stack_mem_end);
    ~Thread() override;

    void Run() override;

    void AdvancePC();

    u64 GetRegX(u8 reg) const override {
        return GetReg((hv_reg_t)(HV_REG_X0 + reg));
    }
    void SetRegX(u8 reg, u64 value) override {
        SetReg((hv_reg_t)(HV_REG_X0 + reg), value);
    }
    u64 GetPC() override { return GetReg(HV_REG_PC); }
    void SetPC(u64 value) override { SetReg(HV_REG_PC, value); }
    u64 GetFP() override { return GetReg(HV_REG_FP); }
    u64 GetLR() override { return GetReg(HV_REG_LR); }
    u64 GetSP() override { return GetSysReg(HV_SYS_REG_SP_EL0); }
    u64 GetElr() override { return GetSysReg(HV_SYS_REG_ELR_EL1); }

    void SetupVTimer();

    void UpdateVTimer();

    // Getters
    u64 GetReg(hv_reg_t reg) const {
        u64 value;
        HV_ASSERT_SUCCESS(hv_vcpu_get_reg(vcpu, reg, &value));

        return value;
    }

    hv_simd_fp_uchar16_t GetRegQ(u8 reg) const {
        hv_simd_fp_uchar16_t value;
        HV_ASSERT_SUCCESS(hv_vcpu_get_simd_fp_reg(
            vcpu, (hv_simd_fp_reg_t)(HV_SIMD_FP_REG_Q0 + reg), &value));

        return value;
    }

    u64 GetSysReg(hv_sys_reg_t reg) const {
        u64 value;
        HV_ASSERT_SUCCESS(hv_vcpu_get_sys_reg(vcpu, reg, &value));

        return value;
    }

    // Setters
    void SetReg(hv_reg_t reg, u64 value) {
        HV_ASSERT_SUCCESS(hv_vcpu_set_reg(vcpu, reg, value));
    }

    void SetRegQ(u8 reg, hv_simd_fp_uchar16_t value) {
        HV_ASSERT_SUCCESS(hv_vcpu_set_simd_fp_reg(
            vcpu, (hv_simd_fp_reg_t)(HV_SIMD_FP_REG_Q0 + reg), value));
    }

    void SetSysReg(hv_sys_reg_t reg, u64 value) {
        HV_ASSERT_SUCCESS(hv_vcpu_set_sys_reg(vcpu, reg, value));
    }

    // Debug
    void LogRegisters(bool simd = false, u32 count = 32) override;

  private:
    hv_vcpu_t vcpu;
    hv_vcpu_exit_t* exit;

    u64 interrupt_time_delta_ticks;

    void InstructionTrap(u32 esr);
    void DataAbort(u32 instruction, u64 far, u64 elr);
};

} // namespace hydra::hw::tegra_x1::cpu::hypervisor
