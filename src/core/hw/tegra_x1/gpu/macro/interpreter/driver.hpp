#pragma once

#include "core/hw/tegra_x1/gpu/macro/driver_base.hpp"

namespace hydra::hw::tegra_x1::gpu::macro::interpreter {

class Driver : public DriverBase {
  public:
    Driver(engines::ThreeD* engine_3d) : DriverBase(engine_3d) {}

  protected:
    void ExecuteImpl(u32 pc_, u32 param1) override;

    u32 InstAlu(AluOperation op, u8 rA, u8 rB) override;
    u32 InstAddImmediate(u8 rA, i32 imm) override;
    u32 InstExtractInsert(u8 bA, u8 rA, u8 bB, u8 rB, u8 size) override;
    u32 InstExtractShiftLeftImmediate(u8 bA, u8 rA, u8 rB, u8 size) override;
    u32 InstExtractShiftLeftRegister(u8 rA, u8 bB, u8 rB, u8 size) override;
    u32 InstRead(u8 rA, u32 imm) override;
    void InstBranch(BranchCondition cond, u8 rA, i32 imm,
                    bool& branched) override;
    void InstResult(ResultOperation op, u8 rD, u32 value) override;

  private:
    u32 pc;
    u32 regs[REG_COUNT] = {0};

    i8 carry = 0;

    u32 branch_after = invalid<u32>();
    u32 branch_addr;

    // Helpers
    u32& GetRegRaw(u8 reg) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);
        return regs[reg];
    }

    u32 GetRegU32(u8 reg) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);
        return GetRegRaw(reg);
    }

    void SetRegU32(u8 reg, u32 value) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);

        if (reg == 0)
            return;

        regs[reg] = value;
    }

    i32 GetRegI32(u8 reg) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);
        return std::bit_cast<i32>(GetRegRaw(reg));
    }

    void SetRegI32(u8 reg, i32 value) {
        ASSERT_DEBUG(reg < REG_COUNT, Macro, "Invalid register {}", reg);

        if (reg == 0)
            return;

        regs[reg] = std::bit_cast<u32>(value);
    }
};

} // namespace hydra::hw::tegra_x1::gpu::macro::interpreter
