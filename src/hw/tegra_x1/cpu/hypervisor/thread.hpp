#pragma once

#include "hw/tegra_x1/cpu/hypervisor/const.hpp"
#include "hw/tegra_x1/cpu/thread_base.hpp"

namespace Hydra::Horizon {
class OS;
}

namespace Hydra::HW::TegraX1::CPU {
class Memory;
}

namespace Hydra::HW::TegraX1::CPU::Hypervisor {

class MMU;
class CPU;

class Thread : public ThreadBase {
  public:
    Thread(MMU* mmu_, MemoryBase* tls_mem);
    ~Thread() override;

    void Configure(const std::function<bool(ThreadBase*, u64)>& svc_handler_,
                   uptr tls_mem_base /*, uptr rom_mem_base*/,
                   uptr stack_mem_end) override;

    void Run() override;

    void AdvancePC();

    u64 GetRegX(u8 reg) const override {
        return GetReg((hv_reg_t)(HV_REG_X0 + reg));
    }

    void SetRegX(u8 reg, u64 value) override {
        SetReg((hv_reg_t)(HV_REG_X0 + reg), value);
    }

    void SetRegPC(u64 value) override { SetReg(HV_REG_PC, value); }

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

    void LogStackTrace(uptr pc);

  private:
    MMU* mmu;

    std::function<bool(ThreadBase*, u64)> svc_handler;

    hv_vcpu_t vcpu;
    hv_vcpu_exit_t* exit;

    u64 interrupt_time_delta_ticks;

    void DataAbort(u32 instruction, u64 far, u64 elr);
};

} // namespace Hydra::HW::TegraX1::CPU::Hypervisor
